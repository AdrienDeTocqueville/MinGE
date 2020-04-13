#include "Renderer/Mesh.h"
#include "Renderer/GLDriver.h"

#include "IO/URI.h"
#include "Utility/Error.h"
#include "Structures/AABB.h"
#include "Structures/MultiArray.h"

#include <vector>

multi_array_t<mesh_t, mesh_data_t, const char*, AABB> mesh_manager;
std::vector<submesh_t> submeshes;

static inline void compute_aabb(const mesh_data_t &data, vec3 &b0, vec3 &b1)
{
	if (data.points)
	{
		b0 = data.points[0];
		b1 = data.points[0];

		for (size_t i = 1; i < data.vertex_count; i++)
		{
			b0 = min(b0, data.points[i]);
			b1 = max(b1, data.points[i]);
		}
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

		glCheck(glBufferData(GL_ARRAY_BUFFER, stride * data.vertex_count, NULL, GL_STATIC_DRAW));
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
		return Mesh(0);

	uint32_t first_submesh = (uint32_t)submeshes.size();

	mesh_data_t data;
	if (uri.on_disk)
	{
		return Mesh(0);
	}
	else
	{
		if (!generate_mesh(uri, data))
			return Mesh(0);

		submeshes.push_back(submesh_t { GL_TRIANGLES, data.index_count, 0 });
	}

	vec3 b0, b1;
	compute_aabb(data, b0, b1);

	uint32_t vao, vbo, ebo;
	load_buffers(data, vao, vbo, ebo);
	for (int i(first_submesh); i < submeshes.size(); i++)
	{
		submeshes[i].vao = vao;
		submeshes[i].vbo = vbo;
		submeshes[i].ebo = ebo;
	}

	uint32_t i = mesh_manager.add();
	mesh_manager.get<0>()[i] = mesh_t {first_submesh, (uint32_t)submeshes.size()};
	mesh_manager.get<1>()[i] = data;
	mesh_manager.get<2>()[i] = URI;
	mesh_manager.get<3>()[i].init(b0, b1);

	return Mesh(i);
}

void Mesh::clear()
{
	for (uint32_t i(0); i < mesh_manager.size; i++)
	{
		submesh_t sub = submeshes[mesh_manager.get<0>()[i].first_submesh];
		GL::DeleteVertexArray(sub.vao);
		GL::DeleteBuffer(sub.vbo);
		GL::DeleteBuffer(sub.ebo);

		mesh_manager.get<1>()[i].free();
	}
	submeshes.clear();
	mesh_manager.clear();
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
