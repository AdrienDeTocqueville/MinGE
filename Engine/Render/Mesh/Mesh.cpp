#include "Render/Mesh/Mesh.h"
#include "Render/GLDriver.h"

#include "IO/URI.h"
#include "IO/DataModel.h"
#include "Utility/Log.h"
#include "Structures/Bounds.h"

array_list_t<Mesh::mesh_t> Mesh::meshes;
array_list_t<submesh_t> Mesh::submeshes;

static inline void destroy_mesh(UID32 object)
{
	Mesh mesh(object);
	ASSERT(mesh.is_valid(), "Trying to destroy an invalid Mesh handle");

	Mesh::mesh_t &data = Mesh::meshes[object.id()];
	Mesh::submeshes_t submeshes = data.submeshes;
	GL::DeleteBuffer(submeshes.vbo);
	GL::DeleteBuffer(submeshes.ebo);

	for (unsigned i = 0; i < submeshes.count; i++)
	{
		submesh_t &submesh = Mesh::submeshes[submeshes.first + i];
		GL::DeleteVertexArray(submesh.vao);
	}

	Mesh::submeshes.remove(submeshes.first, submeshes.count);
	data.data.free();

	data.destroy();
	Mesh::meshes.remove(object.id());
}

void Mesh::destroy()
{
	destroy_mesh(*this);
}


bool generate_mesh(const struct uri_t &uri, mesh_data_t &data);

static inline void compute_aabb(const mesh_data_t &data, vec3 &b0, vec3 &b1)
{
	// TODO: check asm diff when using local variables
	if (data.points)
	{
		vec3 *points = data.points;
		vec3 a = points[0];
		vec3 b = points[0];

		for (size_t i = 1; i < data.vertex_count; i++)
		{
			a = min(a, points[i]);
			b = max(b, points[i]);
		}

		b0 = a;
		b1 = b;
	}
	else b0 = b1 = vec3(0.0f);
}

static inline void load_buffers(const mesh_data_t &data, uint32_t &vao, uint32_t &vbo, uint32_t &ebo)
{
	/// VAO
	vao = GL::GenVertexArray();
	vbo = GL::GenBuffer();
	ebo = GL::GenBuffer();

	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);
	GL::BindElementBuffer(ebo);
	{
		// VBO
		GLsizei stride = data.stride();

		glCheck(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)stride * data.vertex_count, NULL, GL_STATIC_DRAW));
		uint8_t* address = (uint8_t*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		{
			#define UPLOAD(x) if (x) { \
				for (size_t i(0); i < data.vertex_count; i++) \
					memcpy(address + i*stride, x + i, sizeof(*x)); \
				address += sizeof(*x); }

			UPLOAD(data.points);
			UPLOAD(data.normals);
			UPLOAD(data.uvs);
			UPLOAD(data.bones);
		}
		glCheck(glUnmapBuffer(GL_ARRAY_BUFFER));

		// VAO
		#define BIND(slot, stride, x) if (x) { \
			glCheck(glEnableVertexAttribArray(slot)); \
			glCheck(glVertexAttribPointer(slot, sizeof(*x) / sizeof(float), GL_FLOAT, GL_FALSE, stride, address)); \
			address += sizeof(*x); \
			} else { \
			glCheck(glDisableVertexAttribArray(slot)); \
			}

		address = 0;
		BIND(0, stride, data.points);
		BIND(1, stride, data.normals);
		BIND(2, stride, data.uvs);
		if (data.bones)
		{
			glCheck(glEnableVertexAttribArray(3));
			glCheck(glVertexAttribIPointer(3, sizeof(data.bones->bones) / sizeof(int), GL_UNSIGNED_INT, stride, address));
			address += sizeof(data.bones->bones);

			glCheck(glEnableVertexAttribArray(3));
			glCheck(glVertexAttribPointer(4, sizeof(data.bones->weights) / sizeof(float), GL_FLOAT, GL_FALSE, stride, address));
			address += sizeof(data.bones->weights);
		}
		else
		{
			glCheck(glDisableVertexAttribArray(3));
			glCheck(glDisableVertexAttribArray(4));
		}

		// EBO
		glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * data.index_count, data.indices, GL_STATIC_DRAW));
		glCheck(glVertexArrayElementBuffer(vao, ebo));
	}
	GL::BindVertexArray(0);
}

void Mesh::set_mesh_data(mesh_data_t &&data, const submesh_t *submeshes, uint32_t submesh_count)
{
	mesh_t &mesh = Mesh::meshes[id()];

	/*
	if (!generate_mesh(uri, data))
	return Mesh::none;

	submesh_count = 1;
	first_submesh = submeshes.add(submesh_count);
	submeshes[first_submesh] = submesh_t { GL_TRIANGLES, data.index_count, 0 };
	*/

	vec3 b0, b1;
	compute_aabb(data, b0, b1);

	uint32_t vao, vbo, ebo;
	load_buffers(data, vao, vbo, ebo);

	uint32_t first_submesh = Mesh::submeshes.add(submesh_count);
	for (uint32_t i = 0; i < submesh_count; i++)
	{
		Mesh::submeshes[first_submesh + i] = {
			submeshes[i].mode,
			submeshes[i].count,
			submeshes[i].offset,
			vao
		};
	}

	mesh.submeshes = submeshes_t {first_submesh, submesh_count, vbo, ebo};
	mesh.data = std::move(data);
	mesh.aabb.init(b0, b1);
}

Mesh Mesh::create(const char *name)
{
	uint32_t i = meshes.add();
	Mesh::mesh_t &mesh = meshes[i];

	mesh.create();
	if (name != nullptr)
		mesh.name = strdup(name);

	return Mesh::get(i);
}

static UID32 transfer(UID32 object, DataModel *model)
{
	if (!object.valid())
		object = Mesh::create(NULL);
	Mesh asset = Mesh(object);

	Mesh::mesh_t &data = Mesh::meshes[asset.id()];
	model->transfer("name", data.name);

	return object;
}

const object_type_t Mesh::type = []() {
	object_type_t t = INIT_ASSET_TYPE(Mesh);

	t.destroy = destroy_mesh;
	t.transfer = transfer;
	return t;
}();
