#include "Assets/Mesh.h"
#include "Utility/Error.h"

void Submesh::draw() const
{
	//glCheck(glDrawArrays(mode, first, count));
	glCheck(glDrawElements(mode, count, GL_UNSIGNED_SHORT, NULL));
}

Mesh::Mesh(unsigned _dataFlags):
	vao(0), vbo(0), ebo(0),
	dataFlags(_dataFlags)
{ }

Mesh::~Mesh()
{
	glCheck(glDeleteVertexArrays(1, &vao));
	glCheck(glDeleteBuffers(1, &vbo));
	glCheck(glDeleteBuffers(1, &ebo));
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
	glCheck(glDeleteVertexArrays(1, &vao));
	glCheck(glGenVertexArrays(1, &vao));

	/// VBO
	glCheck(glDeleteBuffers(1, &vbo));
	glCheck(glGenBuffers(1, &vbo));

	// EBO
	glCheck(glDeleteBuffers(1, &ebo));
	glCheck(glGenBuffers(1, &ebo));

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
		glCheck(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), indices.data(), GL_STATIC_DRAW));
		glCheck(glVertexArrayElementBuffer(vao, ebo));
	}
	GL::BindVertexArray(0);
}

/// Methods (static)
MeshRef Mesh::createCube(unsigned _dataFlags, vec3 _halfExtent)
{
	const vec3& e = _halfExtent;
	Mesh* m = new Mesh(_dataFlags);

	/*
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
		*/

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

		m->submeshes.push_back(Submesh(GL_TRIANGLES, 0, m->indices.size()));

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

		m->submeshes.push_back(Submesh(GL_TRIANGLES, 0, m->indices.size()));

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
		*/

	m->loadBuffers();

	return MeshRef(m);
}

MeshRef Mesh::createCylinder(unsigned _dataFlags, float _base, float _top, float _height, unsigned _slices)
{
	const float iSlices = 1/((float)_slices);
	Mesh* m = new Mesh(_dataFlags);

	/*
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
		*/

	m->loadBuffers();

	return MeshRef(m);
}

// Model Loader from:
// https://github.com/JoeyDeVries/LearnOpenGL/blob/master/includes/learnopengl/model.h

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static MeshRef processMesh(aiMesh *mesh, const aiScene *scene)
{
	Mesh *res = new Mesh(ALLFLAGS);

	// data to fill
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coordinates
		if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		// tangent
		vector.x = mesh->mTangents[i].x;
		vector.y = mesh->mTangents[i].y;
		vector.z = mesh->mTangents[i].z;
		vertex.Tangent = vector;
		// bitangent
		vector.x = mesh->mBitangents[i].x;
		vector.y = mesh->mBitangents[i].y;
		vector.z = mesh->mBitangents[i].z;
		vertex.Bitangent = vector;
		vertices.push_back(vertex);
	}
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for(unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 2. specular maps
	vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 3. normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	// return a mesh object created from the extracted mesh data
	return MeshRef(res);
}

static MeshRef processNode(aiNode *node, const aiScene *scene)
{
	// process each mesh located at the current node
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
		return processMesh(scene->mMeshes[node->mMeshes[i]], scene);

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		MeshRef mesh = processNode(node->mChildren[i], scene);
		if (mesh)
			return mesh;
	}
	return nullptr;
}

MeshRef Mesh::load(std::string _file)
{
	std::string path = "Resources/" + _file;

       // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
		Error::add(FILE_NOT_FOUND, "Mesh::load() -> " + path);
		return nullptr;
        }

	// Load first mesh
        return processNode(scene->mRootNode, scene);
}
