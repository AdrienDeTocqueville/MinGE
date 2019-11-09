#include "Assets/Mesh.h"

#include "Systems/GraphicEngine.h"
#include "Utility/Error.h"

void Submesh::draw() const
{
	//TODO remove
	glCheck(glDrawElements(mode, count, GL_UNSIGNED_SHORT, offset));
}

Mesh::Mesh(unsigned _dataFlags):
	vao(0), vbo(0), ebo(0),
	dataFlags(_dataFlags)
{ }

Mesh::~Mesh()
{
	GL::DeleteVertexArray(vao);
	GL::DeleteBuffer(vbo);
	GL::DeleteBuffer(ebo);
}

/// Methods (protected)
void Mesh::loadBuffers()
{
	vec3 b0(vertices[0].pos), b1(vertices[0].pos);

	for (size_t i = 1; i < vertices.size(); i++)
	{
		b0 = min(b0, vertices[i].pos);
		b1 = max(b1, vertices[i].pos);
	}
	aabb.init(b0, b1);

	/*
	if (!dataFlags)
		return;

	unsigned hasVertices  = (bool)(dataFlags&VERTICES);
	unsigned hasNormals   = (bool)(dataFlags&NORMALS);
	unsigned hasTexcoords = (bool)(dataFlags&TEXCOORDS);
	*/

	/// VAO
	vao = GL::GenVertexArray();
	vbo = GL::GenBuffer();
	ebo = GL::GenBuffer();

	GL::BindVertexArray(vao);
	GL::BindVertexBuffer(vbo);
	GL::BindElementBuffer(ebo);
	{
		// VBO
		glCheck(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW));

		glCheck(glEnableVertexAttribArray(0));
		glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0));

		glCheck(glEnableVertexAttribArray(1));
		glCheck(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(vec3))));

		glCheck(glEnableVertexAttribArray(2));
		glCheck(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(2*sizeof(vec3))));

		// EBO
		glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW));
		glCheck(glVertexArrayElementBuffer(vao, ebo));
	}
	GL::BindVertexArray(0);
}

/// Methods (static)
MeshRef Mesh::createCube(unsigned _dataFlags, vec3 _halfExtent)
{
	const vec3& e = _halfExtent;
	Mesh* m = new Mesh(_dataFlags);

		m->vertices = {
			// Front Face
			{vec3( e.x, -e.y, -e.z), vec3( 1,  0,  0), vec2(0, 0)},
			{vec3( e.x,  e.y, -e.z), vec3( 1,  0,  0), vec2(0, 1)},
			{vec3( e.x,  e.y,  e.z), vec3( 1,  0,  0), vec2(1, 1)},
			{vec3( e.x, -e.y,  e.z), vec3( 1,  0,  0), vec2(1, 0)},

			// Right Face
			{vec3(-e.x, -e.y, -e.z), vec3( 0, -1,  0), vec2(0, 0)},
			{vec3( e.x, -e.y, -e.z), vec3( 0, -1,  0), vec2(0, 1)},
			{vec3( e.x, -e.y,  e.z), vec3( 0, -1,  0), vec2(1, 1)},
			{vec3(-e.x, -e.y,  e.z), vec3( 0, -1,  0), vec2(1, 0)},

			// Back Face
			{vec3(-e.x,  e.y, -e.z), vec3(-1,  0,  0), vec2(0, 0)},
			{vec3(-e.x, -e.y, -e.z), vec3(-1,  0,  0), vec2(0, 1)},
			{vec3(-e.x, -e.y,  e.z), vec3(-1,  0,  0), vec2(1, 1)},
			{vec3(-e.x,  e.y,  e.z), vec3(-1,  0,  0), vec2(1, 0)},

			// Left Face
			{vec3( e.x,  e.y, -e.z), vec3( 0,  1,  0), vec2(0, 0)},
			{vec3(-e.x,  e.y, -e.z), vec3( 0,  1,  0), vec2(0, 1)},
			{vec3(-e.x,  e.y,  e.z), vec3( 0,  1,  0), vec2(1, 1)},
			{vec3( e.x,  e.y,  e.z), vec3( 0,  1,  0), vec2(1, 0)},

			// Top Face
			{vec3(-e.x, -e.y,  e.z), vec3( 0,  0,  1), vec2(0, 0)},
			{vec3( e.x, -e.y,  e.z), vec3( 0,  0,  1), vec2(0, 1)},
			{vec3( e.x,  e.y,  e.z), vec3( 0,  0,  1), vec2(1, 1)},
			{vec3(-e.x,  e.y,  e.z), vec3( 0,  0,  1), vec2(1, 0)},

			// Bottom Face
			{vec3( e.x, -e.y, -e.z), vec3( 0,  0, -1), vec2(0, 0)},
			{vec3(-e.x, -e.y, -e.z), vec3( 0,  0, -1), vec2(0, 1)},
			{vec3(-e.x,  e.y, -e.z), vec3( 0,  0, -1), vec2(1, 1)},
			{vec3( e.x,  e.y, -e.z), vec3( 0,  0, -1), vec2(1, 0)},

		};
		m->indices = {
			0,  1,  2,  0,  2,  3,
			4,  5,  6,  4,  6,  7,
			8,  9,  10, 8,  10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23,
		};

		m->submeshes.emplace_back(GL_TRIANGLES, m->indices.size());

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createQuad(unsigned _dataFlags, vec2 _halfExtent)
{
	const vec2& e = _halfExtent;
	Mesh* m = new Mesh(_dataFlags);

		m->vertices = {
			{vec3(-e.x, -e.y, 0), vec3(0, 0, 1), vec2(0, 0)},
			{vec3( e.x, -e.y, 0), vec3(0, 0, 1), vec2(1, 0)},
			{vec3( e.x,  e.y, 0), vec3(0, 0, 1), vec2(1, 1)},
			{vec3(-e.x,  e.y, 0), vec3(0, 0, 1), vec2(0, 1)},
		};

		m->indices = {0, 1, 2, 0, 2, 3};

		m->submeshes.emplace_back(GL_TRIANGLES, m->indices.size());

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createSphere(unsigned _dataFlags, float _radius, unsigned _slices, unsigned _stacks)
{
	const float iStacks = 1.0f/(float)(_stacks-1);
	const float iSlices = 1.0f/(float)(_slices-1);
	Mesh* m = new Mesh(_dataFlags);

		vec3 vertex;
		float xy;

		float slice_step = 2 * PI * iSlices;
		float stack_step = PI * iStacks;
		float slice_angle, stack_angle;

		// Generate vertices
		m->vertices.reserve(_slices * _stacks);
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

				m->vertices.push_back({
					vertex * _radius,
					vertex,
					vec2(j*iSlices, i*iStacks)
				});
			}
		}

		// Generate indices
		m->indices.reserve(_slices * _stacks);
		for (int i = 0; i < _stacks; ++i)
		{
			uint16_t k1 = i * (_slices + 1);
			uint16_t k2 = k1 + _slices + 1;

			for (int j = 0; j < _slices; ++j, ++k1, ++k2)
			{
				if (i != 0) {
					m->indices.push_back(k1);
					m->indices.push_back(k2);
					m->indices.push_back(k1 + 1);
				}

				if (i != (_stacks-1)) {
					m->indices.push_back(k1 + 1);
					m->indices.push_back(k2);
					m->indices.push_back(k2 + 1);
				}
			}
		}

		m->submeshes.push_back(Submesh(GL_TRIANGLES, m->indices.size()));

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createCylinder(unsigned _dataFlags, float _base, float _top, float _height, unsigned _slices)
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

// Model Loader from:
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/model.h

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Mesh::Mesh(aiMesh *mesh)
{
	unsigned numVertices = mesh->mNumVertices;
	for (unsigned i = 0; i < numVertices; i++)
	{
		Vertex vertex;

		vertex.pos = vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		);
		vertex.normal = vec3(
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z
		);
		if (mesh->HasTextureCoords(0))
		{
			vertex.texCoords = vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}
		else vertex.texCoords = vec2(0.0f);

		/*
		vertex.tangent = vec3(
			mesh->mTangents[i].x,
			mesh->mTangents[i].y,
			mesh->mTangents[i].z
		);

		vertex.bitangent = vec3(
			mesh->mBitangents[i].x,
			mesh->mBitangents[i].y,
			mesh->mBitangents[i].z
		);
		*/

		vertices.push_back(vertex);
	}

	unsigned numFaces = mesh->mNumFaces;
	for (unsigned i = 0; i < numFaces; i++)
	{
		aiFace &face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	// Must be GL_TRIANGLES since we used triangulate
	submeshes.emplace_back(GL_TRIANGLES, indices.size());

	loadBuffers();
}

static Mesh *processNode(aiNode *node, const aiScene *scene)
{
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
		return new Mesh(scene->mMeshes[node->mMeshes[i]]);

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		if (Mesh *mesh = processNode(node->mChildren[i], scene))
			return mesh;
	}
	return nullptr;
}

MeshRef Mesh::load(std::string _file)
{
	std::string path = "Resources/" + _file;

	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	// check for errors
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		Error::add(FILE_NOT_FOUND, "Mesh::load() -> " + path);
		return nullptr;
	}

	// Load first mesh
	return MeshRef(processNode(scene->mRootNode, scene));
}
