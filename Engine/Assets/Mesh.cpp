#include "Assets/Mesh.h"

void Submesh::draw() const
{
	glCheck(glDrawArrays(mode, first, count));
	//glCheck(glDrawElements(mode, count, GL_UNSIGNED_INT, NULL));
}

Mesh::Mesh(unsigned _dataFlags):
	vbo(0), vao(0), //ebo(0),
	dataFlags(_dataFlags)
{ }

Mesh::~Mesh()
{
	glCheck(glDeleteBuffers(1, &vbo));
	glCheck(glDeleteVertexArrays(1, &vao));
	//glCheck(glDeleteBuffers(1, &ebo));
}

/// Methods (protected)
void Mesh::loadBuffers()
{
	aabb.compute(vertices);

	if (!dataFlags)
		return;

	unsigned hasVertices  = (bool)(dataFlags&VERTICES);
	unsigned hasNormals   = (bool)(dataFlags&NORMALS);
	unsigned hasTexcoords = (bool)(dataFlags&TEXCOORDS);

	unsigned dataSize[3];
	dataSize[0] = vertices.size() *sizeof(vec3);
	dataSize[1] = normals.size()  *sizeof(vec3);
	dataSize[2] = texCoords.size()*sizeof(vec2);

	unsigned offset[3];
	offset[0] = dataSize[0] * hasVertices;
	offset[1] = dataSize[1] * hasNormals   + offset[0];
	offset[2] = dataSize[2] * hasTexcoords + offset[1];;

	/// VBO
	glCheck(glDeleteBuffers(1, &vbo));
	glCheck(glGenBuffers(1, &vbo));

	GL::BindVertexBuffer(vbo);

		glCheck(glBufferData(GL_ARRAY_BUFFER, offset[2], nullptr, GL_STATIC_DRAW));

		if (hasVertices)
			glCheck(glBufferSubData(GL_ARRAY_BUFFER, 0        , dataSize[0], &vertices [0]));

		if (hasNormals)
			glCheck(glBufferSubData(GL_ARRAY_BUFFER, offset[0], dataSize[1], &normals  [0]));

		if (hasTexcoords)
			glCheck(glBufferSubData(GL_ARRAY_BUFFER, offset[1], dataSize[2], &texCoords[0]));

	/*
	// EBO
	glCheck(glDeleteBuffers(1, &ebo));
	glCheck(glGenBuffers(1, &ebo));

	glCheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo));
	glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uvec3), indices.data(), GL_STATIC_DRAW));
	*/

	/// VAO
	glCheck(glDeleteVertexArrays(1, &vao));
	glCheck(glGenVertexArrays(1, &vao));

	GL::BindVertexArray(vao);

		if (hasVertices)
		{
			glCheck(glEnableVertexAttribArray(0));
			glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)));
		}

		if (hasNormals)
		{
			glCheck(glEnableVertexAttribArray(hasVertices));
			glCheck(glVertexAttribPointer(hasVertices, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset[0])));
		}

		if (hasTexcoords)
		{
			glCheck(glEnableVertexAttribArray(hasVertices+hasNormals));
			glCheck(glVertexAttribPointer(hasVertices+hasNormals, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset[1])));
		}
}

/// Methods (static)
MeshRef Mesh::createCube(unsigned _dataFlags, vec3 _halfExtent)
{
	const vec3& e = _halfExtent;
	Mesh* m = new Mesh(_dataFlags);

		m->vertices =
		{
			vec3( e.x, -e.y, -e.z), vec3( e.x,  e.y, -e.z), vec3( e.x,  e.y,  e.z), vec3( e.x, -e.y,  e.z), // Front
			vec3(-e.x, -e.y,  e.z), vec3(-e.x,  e.y,  e.z), vec3(-e.x,  e.y, -e.z), vec3(-e.x, -e.y, -e.z), // Back

			vec3( e.x,  e.y, -e.z), vec3(-e.x,  e.y, -e.z), vec3(-e.x,  e.y,  e.z), vec3( e.x,  e.y,  e.z), // Right
			vec3(-e.x, -e.y, -e.z), vec3( e.x, -e.y, -e.z), vec3( e.x, -e.y,  e.z), vec3(-e.x, -e.y,  e.z), // Left

			vec3( e.x, -e.y,  e.z), vec3( e.x,  e.y,  e.z), vec3(-e.x,  e.y,  e.z), vec3(-e.x, -e.y,  e.z), // Top
			vec3(-e.x, -e.y, -e.z), vec3(-e.x,  e.y, -e.z), vec3( e.x,  e.y, -e.z), vec3( e.x, -e.y, -e.z)  // Bottom
		};

		m->normals =
		{
			vec3( 1,  0,  0), vec3( 1,  0,  0), vec3( 1,  0,  0), vec3( 1,  0,  0),
			vec3(-1,  0,  0), vec3(-1,  0,  0), vec3(-1,  0,  0), vec3(-1,  0,  0),

			vec3( 0,  1,  0), vec3( 0,  1,  0), vec3( 0,  1,  0), vec3( 0,  1,  0),
			vec3( 0, -1,  0), vec3( 0, -1,  0), vec3( 0, -1,  0), vec3( 0, -1,  0),

			vec3( 0,  0,  1), vec3( 0,  0,  1), vec3( 0,  0,  1), vec3( 0,  0,  1),
			vec3( 0,  0, -1), vec3( 0,  0, -1), vec3( 0,  0, -1), vec3( 0,  0, -1)
		};

		m->texCoords =
		{
			vec2(1, 0), vec2(0, 0), vec2(0, 1), vec2(1, 1),
			vec2(0, 1), vec2(1, 1), vec2(1, 0), vec2(0, 0),

			vec2(1, 0), vec2(0, 0), vec2(0, 1), vec2(1, 1),
			vec2(1, 0), vec2(0, 0), vec2(0, 1), vec2(1, 1),

			vec2(1, 1), vec2(0, 1), vec2(0, 0), vec2(1, 0),
			vec2(1, 0), vec2(0, 0), vec2(0, 1), vec2(1, 1)
		};

		m->submeshes.push_back(Submesh(GL_QUADS, 0, m->vertices.size()));

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createQuad(unsigned _dataFlags, vec2 _halfExtent)
{
	const vec2& e = _halfExtent;
	Mesh* m = new Mesh(_dataFlags);

		m->vertices = { vec3(-e.x, -e.y, 0), vec3(e.x, -e.y, 0), vec3(e.x, e.y, 0), vec3(-e.x, e.y, 0) };

		m->normals = { vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1), vec3(0, 0, 1) };

		m->texCoords = { vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1) };

		m->submeshes.push_back(Submesh(GL_QUADS, 0, m->vertices.size()));

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createSphere(unsigned _dataFlags, float _radius, unsigned _slices, unsigned _stacks)
{
	const float iStacks = 1.0f/(float)(_stacks-1);
	const float iSlices = 1.0f/(float)(_slices-1);
	Mesh* m = new Mesh(_dataFlags);

		/*
		vec3 vertex;
		float xy;

		float slice_step = 2 * PI * iSlices;
		float stack_step = PI * iStacks;
		float slice_angle, stack_angle;

		m->vertices.reserve(_slices * _stacks);
		m->normals.reserve(_slices * _stacks);
		m->texCoords.reserve(_slices * _stacks);
		m->indices.reserve(_slices * _stacks);

		for (int i = 0; i <= _stacks; ++i)
		{
			stack_angle = 0.5f * PI - i * stack_step;
			xy = cosf(stack_angle);
			vertex.z = sinf(stack_angle);

			for (int j = 0; j <= _slices; ++j)
			{
				slice_angle = j * slice_step;

				vertex.x = xy * cosf(slice_angle);
				vertex.y = xy * sinf(slice_angle);

				m->vertices.push_back(vertex * _radius);
				m->normals.push_back(vertex);
				m->texCoords.emplace_back(j*iSlices, i*iStacks);
			}
		}

		uint32_t k1, k2;
		for (int i = 0; i < _stacks; ++i)
		{
			k1 = i * (_slices + 1);
			k2 = k1 + _slices + 1;

			for (int j = 0; j < _slices; ++j, ++k1, ++k2)
			{
				if (i != 0)
					m->indices.emplace_back(k1, k2, k1 + 1);

				if (i != (_stacks-1))
					m->indices.emplace_back(k1 + 1, k2, k2 + 1);
			}
		}

		m->submeshes.push_back(Submesh(GL_TRIANGLES, 0, m->indices.size() * 3));
		*/

		std::vector<vec3> vertices(_slices * _stacks);
		std::vector<vec3> normals(_slices * _stacks);
		std::vector<vec2> texCoords(_slices * _stacks);

		unsigned i(0);

		for(unsigned r(0) ; r < _stacks ; r++)
		{
			for(unsigned s(0) ; s < _slices ; s++)
			{
				float const x = cos(2.0f*PI * s * iSlices) * sin( PI * r * iStacks );
				float const y = sin(2.0f*PI * s * iSlices) * sin( PI * r * iStacks );
				float const z = sin( -PI*0.5f + PI * r * iStacks );

				texCoords[i] = vec2(s*iSlices, r*iStacks);
				vertices[i]  = _radius * vec3(x, y, z);
				normals[i]   = vec3(x, y, z);

				i++;
			}
		}

		i = 0;
		m->vertices.resize(4*vertices.size());
		m->normals.resize(4*normals.size());
		m->texCoords.resize(4*texCoords.size());

		for(unsigned r(0) ; r < _stacks-1 ; r++)
		{
			for(unsigned s(0) ; s < _slices-1 ; s++)
			{
				m->vertices[i]   = vertices[r * _slices + s];
				m->vertices[i+1] = vertices[r * _slices + s+1];
				m->vertices[i+2] = vertices[(r+1) * _slices + s+1];
				m->vertices[i+3] = vertices[(r+1) * _slices + s];

				m->normals[i]   = normals[r * _slices + s];
				m->normals[i+1] = normals[r * _slices + s+1];
				m->normals[i+2] = normals[(r+1) * _slices + s+1];
				m->normals[i+3] = normals[(r+1) * _slices + s];

				m->texCoords[i]   = texCoords[r * _slices + s];
				m->texCoords[i+1] = texCoords[r * _slices + s+1];
				m->texCoords[i+2] = texCoords[(r+1) * _slices + s+1];
				m->texCoords[i+3] = texCoords[(r+1) * _slices + s];

				i += 4;
			}
		}

		m->submeshes.push_back(Submesh(GL_QUADS, 0, m->vertices.size()));

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createCylinder(unsigned _dataFlags, float _base, float _top, float _height, unsigned _slices)
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
