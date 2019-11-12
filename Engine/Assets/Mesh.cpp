#include "Assets/Mesh.h"

#include "Systems/GraphicEngine.h"
#include "Utility/Error.h"

MeshData::MeshData(uint32_t _vertex_count, uint32_t _index_count, MeshData::Flags _flags):
	vertex_count(_vertex_count), index_count(_index_count),
	points(NULL), normals(NULL), uvs(NULL), bones(NULL),
	indices(NULL)
{
	if (_flags & Flags::Points)
		points = new vec3[vertex_count];
	if (_flags & Flags::Normals)
		normals = new vec3[vertex_count];
	if (_flags & Flags::UVs)
		uvs = new vec2[vertex_count];
	if (_flags & Flags::Bones)
		bones = new BoneWeight[vertex_count];

	indices = new uint16_t[index_count];
}

MeshData::MeshData(MeshData &&data)
{
	memcpy(this, &data, sizeof(MeshData));
	memset(&data, 0, sizeof(MeshData));
}

MeshData::~MeshData()
{
	delete[] points;
	delete[] normals;
	delete[] uvs;
	delete[] bones;

	delete[] indices;
}

uint32_t MeshData::stride() const
{
	uint32_t stride = 0;
	if (points)	stride += sizeof(*points);
	if (normals)	stride += sizeof(*normals);
	if (uvs)	stride += sizeof(*uvs);
	if (bones)	stride += sizeof(*bones);

	return stride;
}

Mesh::Mesh(MeshData &&_data, const std::vector<Submesh> &_submeshes):
	submeshes(_submeshes), data(std::move(_data)),
	vao(0), vbo(0), ebo(0)
{
	if (data.points)
	{
		vec3 b0(data.points[0]), b1(data.points[0]);

		for (size_t i = 1; i < data.vertex_count; i++)
		{
			b0 = min(b0, data.points[i]);
			b1 = max(b1, data.points[i]);
		}
		aabb.init(b0, b1);
	}

	loadBuffers();
}

Mesh::~Mesh()
{
	GL::DeleteVertexArray(vao);
	GL::DeleteBuffer(vbo);
	GL::DeleteBuffer(ebo);
}

/// Methods (protected)
void Mesh::loadBuffers()
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
				for (int i(0); i < data.vertex_count; i++) \
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

/// Methods (static)
MeshRef Mesh::createCube(MeshData::Flags flags, vec3 halfExtent)
{
	if (flags & MeshData::Bones)
		return nullptr;

	const vec3& e = halfExtent;
	MeshData data(4 * 6, 6 * 6, flags);

	if (flags & MeshData::Points)
	{
		size_t i(0);
		// Front Face
		data.points[i++] = vec3( e.x, -e.y, -e.z);
		data.points[i++] = vec3( e.x,  e.y, -e.z);
		data.points[i++] = vec3( e.x,  e.y,  e.z);
		data.points[i++] = vec3( e.x, -e.y,  e.z);

		// Right Face
		data.points[i++] = vec3(-e.x, -e.y, -e.z);
		data.points[i++] = vec3( e.x, -e.y, -e.z);
		data.points[i++] = vec3( e.x, -e.y,  e.z);
		data.points[i++] = vec3(-e.x, -e.y,  e.z);

		// Back Face
		data.points[i++] = vec3(-e.x,  e.y, -e.z);
		data.points[i++] = vec3(-e.x, -e.y, -e.z);
		data.points[i++] = vec3(-e.x, -e.y,  e.z);
		data.points[i++] = vec3(-e.x,  e.y,  e.z);

		// Left Face
		data.points[i++] = vec3( e.x,  e.y, -e.z);
		data.points[i++] = vec3(-e.x,  e.y, -e.z);
		data.points[i++] = vec3(-e.x,  e.y,  e.z);
		data.points[i++] = vec3( e.x,  e.y,  e.z);

		// Top Face
		data.points[i++] = vec3(-e.x, -e.y,  e.z);
		data.points[i++] = vec3( e.x, -e.y,  e.z);
		data.points[i++] = vec3( e.x,  e.y,  e.z);
		data.points[i++] = vec3(-e.x,  e.y,  e.z);

		// Bottom Face
		data.points[i++] = vec3( e.x, -e.y, -e.z);
		data.points[i++] = vec3(-e.x, -e.y, -e.z);
		data.points[i++] = vec3(-e.x,  e.y, -e.z);
		data.points[i++] = vec3( e.x,  e.y, -e.z);
	}
	if (flags & MeshData::Normals)
	{
		size_t i(0);
		// Front Face
		data.normals[i++] = vec3( 1,  0,  0);
		data.normals[i++] = vec3( 1,  0,  0);
		data.normals[i++] = vec3( 1,  0,  0);
		data.normals[i++] = vec3( 1,  0,  0);

		// Right Face
		data.normals[i++] = vec3( 0, -1,  0);
		data.normals[i++] = vec3( 0, -1,  0);
		data.normals[i++] = vec3( 0, -1,  0);
		data.normals[i++] = vec3( 0, -1,  0);

		// Back Face
		data.normals[i++] = vec3(-1,  0,  0);
		data.normals[i++] = vec3(-1,  0,  0);
		data.normals[i++] = vec3(-1,  0,  0);
		data.normals[i++] = vec3(-1,  0,  0);

		// Left Face
		data.normals[i++] = vec3( 0,  1,  0);
		data.normals[i++] = vec3( 0,  1,  0);
		data.normals[i++] = vec3( 0,  1,  0);
		data.normals[i++] = vec3( 0,  1,  0);

		// Top Face
		data.normals[i++] = vec3( 0,  0,  1);
		data.normals[i++] = vec3( 0,  0,  1);
		data.normals[i++] = vec3( 0,  0,  1);
		data.normals[i++] = vec3( 0,  0,  1);

		// Bottom Face
		data.normals[i++] = vec3( 0,  0, -1);
		data.normals[i++] = vec3( 0,  0, -1);
		data.normals[i++] = vec3( 0,  0, -1);
		data.normals[i++] = vec3( 0,  0, -1);
	}
	if (flags & MeshData::UVs)
	{
		size_t i(0);
		// Front Face
		data.uvs[i++] = vec2(0, 0);
		data.uvs[i++] = vec2(0, 1);
		data.uvs[i++] = vec2(1, 1);
		data.uvs[i++] = vec2(1, 0);

		// Right Face
		data.uvs[i++] = vec2(0, 0);
		data.uvs[i++] = vec2(0, 1);
		data.uvs[i++] = vec2(1, 1);
		data.uvs[i++] = vec2(1, 0);

		// Back Face
		data.uvs[i++] = vec2(0, 0);
		data.uvs[i++] = vec2(0, 1);
		data.uvs[i++] = vec2(1, 1);
		data.uvs[i++] = vec2(1, 0);

		// Left Face
		data.uvs[i++] = vec2(0, 0);
		data.uvs[i++] = vec2(0, 1);
		data.uvs[i++] = vec2(1, 1);
		data.uvs[i++] = vec2(1, 0);

		// Top Face
		data.uvs[i++] = vec2(0, 0);
		data.uvs[i++] = vec2(0, 1);
		data.uvs[i++] = vec2(1, 1);
		data.uvs[i++] = vec2(1, 0);

		// Bottom Face
		data.uvs[i++] = vec2(0, 0);
		data.uvs[i++] = vec2(0, 1);
		data.uvs[i++] = vec2(1, 1);
		data.uvs[i++] = vec2(1, 0);
	}

	static const uint16_t indices[] = {
		0,  1,  2,  0,  2,  3,
		4,  5,  6,  4,  6,  7,
		8,  9,  10, 8,  10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
	};
	memcpy(data.indices, indices, sizeof(indices));

	return MeshRef(new Mesh(std::move(data), {{GL_TRIANGLES, data.index_count}}));
}

MeshRef Mesh::createQuad(MeshData::Flags flags, vec2 halfExtent, uvec2 subdiv, vec2 tiling)
{
	if (flags & MeshData::Bones)
		return nullptr;

	const vec2& e = halfExtent;
	MeshData data(subdiv.x * subdiv.y, (subdiv.x - 1) * (subdiv.y - 1) * 6, flags);

	size_t v(0);
	for (int i = 0; i < subdiv.y; i++)
	{
		for (int j = 0; j < subdiv.x; j++)
		{
			vec2 uv(
				float(j) / (subdiv.x - 1),
				float(i) / (subdiv.y - 1)
			);

			if (flags & MeshData::Points)
				data.points[v] = vec3(
					e.x * (uv.x * 2.0f - 1.0f),
					e.y * (uv.y * 2.0f - 1.0f),
					0.0f
				);
			if (flags & MeshData::Normals)
				data.normals[v] = vec3(0, 0, 1);
			if (flags & MeshData::UVs)
				data.uvs[v] = uv * tiling;
			v++;
		}
	}

	v = 0;
	for (int i = 0; i < subdiv.y - 1; i++)
	{
		for (int j = 0; j < subdiv.x - 1; j++)
		{
			data.indices[v++] = j + 0 + subdiv.x * (i + 0);
			data.indices[v++] = j + 1 + subdiv.x * (i + 0);
			data.indices[v++] = j + 0 + subdiv.x * (i + 1);

			data.indices[v++] = j + 1 + subdiv.x * (i + 0);
			data.indices[v++] = j + 1 + subdiv.x * (i + 1);
			data.indices[v++] = j + 0 + subdiv.x * (i + 1);
		}
	}

	return MeshRef(new Mesh(std::move(data), {{GL_TRIANGLES, data.index_count}}));
}

MeshRef Mesh::createSphere(MeshData::Flags flags, float radius, unsigned slices, unsigned stacks)
{
	if (flags & MeshData::Bones)
		return nullptr;

	const float iStacks = 1.0f/(float)(stacks-1);
	const float iSlices = 1.0f/(float)(slices-1);
	MeshData data((slices+1) * (stacks+1), 6 * slices * (stacks-1), flags);

	float slice_step = 2 * PI * iSlices;
	float stack_step = PI * iStacks;
	float slice_angle, stack_angle;

	// Generate vertices
	size_t v(0);
	for (int i = 0; i <= stacks; ++i)
	{
		stack_angle = 0.5f * PI - i * stack_step;
		float xy = cosf(stack_angle);
		float z = sinf(stack_angle);

		for (int j = 0; j <= slices; ++j)
		{
			slice_angle = j * slice_step;

			vec3 normal(xy * cosf(slice_angle), xy * sinf(slice_angle), z);

			if (flags & MeshData::Points)
				data.points[v] = normal * radius;
			if (flags & MeshData::Normals)
				data.normals[v] = normal;
			if (flags & MeshData::UVs)
				data.uvs[v] = vec2(j*iSlices, i*iStacks);
			v++;
		}
	}

	// Generate indices
	v = 0;
	for (int i = 0; i < stacks; ++i)
	{
		uint16_t k1 = i * (slices + 1);
		uint16_t k2 = k1 + slices + 1;

		for (int j = 0; j < slices; ++j, ++k1, ++k2)
		{
			if (i != 0) {
				data.indices[v++] = k1;
				data.indices[v++] = k2;
				data.indices[v++] = k1 + 1;
			}

			if (i != (stacks-1)) {
				data.indices[v++] = k1 + 1;
				data.indices[v++] = k2;
				data.indices[v++] = k2 + 1;
			}
		}
	}

	return MeshRef(new Mesh(std::move(data), {{GL_TRIANGLES, data.index_count}}));
}

MeshRef Mesh::createCylinder(MeshData::Flags flags, float _base, float _top, float _height, unsigned _slices)
{
	return nullptr;
	/*
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
	*/
}
