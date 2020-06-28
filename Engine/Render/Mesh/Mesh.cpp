#include "Render/Mesh/Mesh.h"
#include "Render/GLDriver.h"

#include "IO/URI.h"
#include "IO/json.hpp"
#include "Utility/Error.h"
#include "Structures/Bounds.h"


const Mesh Mesh::none;
// submeshes, mesh data, URI, AABB, generation
multi_array_t<submeshes_t, mesh_data_t, char*, AABB, uint8_t> Mesh::meshes;
array_list_t<submesh_t> Mesh::submeshes;


void Mesh::destroy()
{
	assert(is_valid() && "Invalid Mesh handle");

	submeshes_t subs = *meshes.get<0>(id());
	submesh_t sub = submeshes[subs.first];

	GL::DeleteVertexArray(sub.vao);
	GL::DeleteBuffer(subs.vbo);
	GL::DeleteBuffer(subs.ebo);
	submeshes.remove(subs.first, subs.count);
	
	meshes.get<1>(id())->free();
	free(meshes.get<2>()[id()]);
	*meshes.get<2>(id()) = NULL;
	++(*meshes.get<4>(id()));

	// Return to pool, generation will be
	// kept if the slot gets recycled
	meshes.remove(id());
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

			glCheck(glEnableVertexAttribArray(4));
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

Mesh Mesh::load(const char *URI)
{
	uri_t uri;
	if (!uri.parse(URI))
		return Mesh::none;

	uint32_t first_submesh, submesh_count;

	mesh_data_t data;
	if (uri.on_disk)
	{
		// remember that submesh_t::offset = _first_index * sizeof(uint16_t)
		return Mesh::none;
	}
	else
	{
		if (!generate_mesh(uri, data))
			return Mesh::none;

		submesh_count = 1;
		first_submesh = submeshes.add(1);
		submeshes[first_submesh] = submesh_t { GL_TRIANGLES, data.index_count, 0 };
	}

	vec3 b0, b1;
	compute_aabb(data, b0, b1);

	uint32_t vao, vbo, ebo;
	load_buffers(data, vao, vbo, ebo);

	uint32_t last_submesh = first_submesh + submesh_count;
	for (uint32_t i = first_submesh; i < last_submesh; i++)
		submeshes[i].vao = vao;

	uint32_t i = meshes.add();
	meshes.get<0>()[i] = submeshes_t {first_submesh, submesh_count, vbo, ebo};
	meshes.get<1>()[i] = data;
	meshes.get<2>()[i] = strdup(URI);
	meshes.get<3>()[i].init(b0, b1);

	return Mesh(i, meshes.get<4>()[i]);
}

Mesh Mesh::get(uint32_t i)
{
	if (meshes.get<2>()[i] == NULL)
		return Mesh::none;
	return Mesh(i, meshes.get<4>()[i]);
}

void Mesh::clear()
{
	for (uint32_t i(1); i <= meshes.size; i++)
	{
		if (meshes.get<2>()[i] == NULL)
			continue;

		submeshes_t subs = *meshes.get<0>(i);
		submesh_t sub = submeshes[subs.first];

		GL::DeleteVertexArray(sub.vao);
		GL::DeleteBuffer(subs.vbo);
		GL::DeleteBuffer(subs.ebo);
		free(meshes.get<2>()[i]);

		meshes.get<1>(i)->free();
	}
	submeshes.clear();
	meshes.clear();
}


/// Serialization
using namespace nlohmann;
void mesh_save(json &dump)
{
	uint32_t max_id = 0;
	json meshes = json::array();
	meshes.get_ptr<json::array_t*>()->reserve(Mesh::meshes.size);
	for (uint32_t i(1); i <= Mesh::meshes.size; i++)
	{
		auto mesh = Mesh::get(i);
		if (mesh == Mesh::none)
			continue;

		max_id = i;
		json mesh_dump = json::object();
		mesh_dump["uint"] = mesh.uint();
		mesh_dump["uri"] = mesh.uri();
		meshes.push_back(mesh_dump);
	}

	dump["max_id"] = max_id;
	dump["meshes"].swap(meshes);
}

void mesh_load(const json &dump)
{
	uint32_t final_slot = 1;
	uint32_t max_id = dump["max_id"].get<uint32_t>();

	// Clear free list
	Mesh::meshes.init(max_id);
	for (uint32_t i(1); i <= max_id; i++)
		Mesh::meshes.get<2>()[i] = NULL;

	// Populate
	const json &meshes = dump["meshes"];
	for (auto it = meshes.rbegin(); it != meshes.rend(); ++it)
	{
		UID32 uid = it.value()["uint"].get<uint32_t>();

		auto *data = Mesh::meshes.get<0>();
		if (uid.id() == 1) final_slot = *(uint32_t*)(data + uid.id());
		else *(uint32_t*)(data + uid.id() - 1) = *(uint32_t*)(data + uid.id());

		Mesh::meshes.next_slot = uid.id();
		Mesh::load(it.value()["uri"].get<std::string>().c_str());
		Mesh::meshes.get<4>()[uid.id()] = uid.gen();
	}
	Mesh::meshes.next_slot = final_slot;
}

const asset_type_t Mesh::type = []() {
	asset_type_t t{ NULL };
	t.name = "Mesh";

	t.save = mesh_save;
	t.load = mesh_load;
	return t;
}();
