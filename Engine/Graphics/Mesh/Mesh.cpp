#include "Graphics/Mesh/Mesh.h"
#include "Graphics/GLDriver.h"

#include "IO/URI.h"
#include "Utility/Error.h"
#include "Structures/Bounds.h"


const Mesh Mesh::none;
multi_array_t<submeshes_t, mesh_data_t, const char*, AABB, uint8_t> Mesh::meshes;
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
		//glCheck(glVertexArrayElementBuffer(vao, ebo));
	}
	GL::BindVertexArray(0);
}

Mesh Mesh::import(const char *URI)
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
	for (int i = first_submesh; i < last_submesh; i++)
		submeshes[i].vao = vao;

	uint32_t prev_size = meshes.size;
	uint32_t i = meshes.add();
	meshes.get<0>()[i] = submeshes_t {first_submesh, submesh_count, vbo, ebo};
	meshes.get<1>()[i] = data;
	meshes.get<2>()[i] = URI;
	meshes.get<3>()[i].init(b0, b1);

	// Only initialize generation if new memory is allocated
	// If a slot is recycled, we got to keep the generation index
	if (prev_size != meshes.size)
		meshes.get<4>()[i] = 0;

	return Mesh(i, meshes.get<4>()[i]);
}

void Mesh::clear()
{
	for (uint32_t i(1); i <= meshes.size; i++)
	{
		if (meshes.get<2>(i) == NULL)
			continue;

		submeshes_t subs = *meshes.get<0>(i);
		submesh_t sub = submeshes[subs.first];

		GL::DeleteVertexArray(sub.vao);
		GL::DeleteBuffer(subs.vbo);
		GL::DeleteBuffer(subs.ebo);

		meshes.get<1>(i)->free();
	}
	submeshes.clear();
	meshes.clear();
}


/*
MeshRef Mesh::createCylinder(mesh_data_t::Flags flags, float _base, float _top, float _height, unsigned _slices)
{
	const float iSlices = 1/((float)_slices);
	Mesh* m = new Mesh(_dataFlags);

		float b = (_base == _top)? 0.5f : 0.25f;
		const vec3 base = vec3(0, 0, -_height * b);
		const vec3 top  = vec3(0, 0,  _height * (1.0f - b));

		std::vector<vec3> vertices(_slices+1);

		bool useBase = _base != 0.0f;
		bool useTop  = _top != 0.0f;

		for (unsigned x(0) ; x <= _slices ; x++)
		{
			float angle = x*2.0f*PI * iSlices;

			vertices[x] = vec3(cos(angle), sin(angle), 0);
		}

		unsigned size = _slices * 3 * (useBase + useTop + 2);
		m->vertices.reserve(size);
		m->normals.reserve(size);
		m->texCoords.reserve(size);

		for (unsigned i(0) ; i < _slices ; i++)
		{
			if (useBase)
			{
				m->vertices.push_back(_base*vertices[i+1] + base);
				m->vertices.push_back(_base*vertices[i] + base);
				m->vertices.push_back(base);

				m->normals.push_back(vec3(0, 0, -1));
				m->normals.push_back(vec3(0, 0, -1));
				m->normals.push_back(vec3(0, 0, -1));

				m->texCoords.push_back(vec2(vertices[i+1]*0.5f+0.5f));
				m->texCoords.push_back(vec2(vertices[i]*0.5f+0.5f));
				m->texCoords.push_back(vec2(0.5f));
			}
			if (useTop)
			{
				m->vertices.push_back(top);
				m->vertices.push_back(_top*vertices[i] + top);
				m->vertices.push_back(_top*vertices[i+1] + top);

				m->normals.push_back(vec3(0, 0, 1));
				m->normals.push_back(vec3(0, 0, 1));
				m->normals.push_back(vec3(0, 0, 1));

				m->texCoords.push_back(vec2(0.5f));
				m->texCoords.push_back(vec2(vertices[i]*0.5f+0.5f));
				m->texCoords.push_back(vec2(vertices[i+1]*0.5f+0.5f));
			}

			m->vertices.push_back(_top*vertices[i] + top);
			m->vertices.push_back(_base*vertices[i] + base);
			m->vertices.push_back(_base*vertices[i+1] + base);

			m->normals.push_back(vertices[i]);
			m->normals.push_back(vertices[i]);
			m->normals.push_back(vertices[i+1]);

				m->texCoords.push_back(vec2(vertices[i]*0.5f+0.5f));
				m->texCoords.push_back(vec2(vertices[i]*0.5f+0.5f));
				m->texCoords.push_back(vec2(vertices[i+1]*0.5f+0.5f));


			m->vertices.push_back(_base*vertices[i+1] + base);
			m->vertices.push_back(_top*vertices[i+1] + top);
			m->vertices.push_back(_top*vertices[i] + top);

			m->normals.push_back(vertices[i+1]);
			m->normals.push_back(vertices[i+1]);
			m->normals.push_back(vertices[i]);

			m->texCoords.push_back(vec2(0.0f));
			m->texCoords.push_back(vec2(0.0f));
			m->texCoords.push_back(vec2(0.0f));
		}

		m->submeshes.push_back(Submesh(GL_TRIANGLES, 0, m->vertices.size()));

	m->loadBuffers();

	return MeshRef(m);
}
*/
