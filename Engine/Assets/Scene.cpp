#include "Assets/Scene.h"

#include "MinGE.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static MeshRef import_mesh(aiNode *node, aiMesh **_submeshes)
{
	std::vector<Submesh> submeshes;
	submeshes.reserve(node->mNumMeshes);

	// Alloc mesh data
	int vertex_count = 0, index_count = 0;
	for (unsigned sub = 0; sub < node->mNumMeshes; sub++)
	{
		aiMesh *submesh = _submeshes[node->mMeshes[sub]];
		unsigned numVertices = submesh->mNumVertices;
		unsigned numIndices = submesh->mNumFaces * 3;

		submeshes.emplace_back(GL_TRIANGLES, numIndices, index_count);
		vertex_count += numVertices;
		index_count += numIndices;
	}

	MeshData data(vertex_count, index_count, MeshData::Basic);

	// Read data
	vertex_count = index_count = 0;
	for (unsigned sub = 0; sub < node->mNumMeshes; sub++)
	{
		aiMesh *submesh = _submeshes[node->mMeshes[sub]];
		unsigned numVertices = submesh->mNumVertices;
		unsigned numFaces = submesh->mNumFaces;

		// Read vertices
		for (unsigned i = 0; i < numVertices; i++)
		{
			size_t v = vertex_count + i;

			data.points[v] = vec3(
				submesh->mVertices[i].x,
				submesh->mVertices[i].y,
				submesh->mVertices[i].z
			);
			data.normals[v] = vec3(
				submesh->mNormals[i].x,
				submesh->mNormals[i].y,
				submesh->mNormals[i].z
			);
			if (submesh->HasTextureCoords(0))
			{
				data.uvs[v] = vec2(
					submesh->mTextureCoords[0][i].x,
					submesh->mTextureCoords[0][i].y
				);
			}
			else data.uvs[v] = vec2(0.0f);
		}

		// Read indices
		for (unsigned i = 0; i < numFaces; i++)
		{
			auto *face = submesh->mFaces[i].mIndices;
			for (unsigned j = 0; j < 3; j++)
				data.indices[index_count++] = vertex_count + face[j];
		}

		vertex_count += numVertices;
	}

	return MeshRef(new Mesh(std::move(data), submeshes));
}

static void import_node(const aiScene *scene, aiNode *node, std::vector<MaterialRef> &materials)
{
	if (node->mNumMeshes)
	{
		MeshRef mesh = import_mesh(node, scene->mMeshes);
		std::vector<MaterialRef> mats;
		for (unsigned sub = 0; sub < node->mNumMeshes; sub++)
		{
			aiMesh *submesh = scene->mMeshes[node->mMeshes[sub]];
			mats.push_back(materials[submesh->mMaterialIndex]);
		}

		Entity::create("import", false, vec3(0.0f), vec3(0,0,3.14f))
			->insert<Graphic>(mesh, mats);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		import_node(scene, node->mChildren[i], materials);
}

Entity *Scene::import(const std::string &file)
{
	std::string path = "Resources/" + file;

	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Error::add(FILE_NOT_FOUND, "Scene::import() -> " + path);
		return nullptr;
	}

	// Load materials
	std::vector<MaterialRef> materials;
	for (unsigned i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial *mat = scene->mMaterials[i];

		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		std::string diff(path.C_Str());

		MaterialRef m = Material::getDefault()->clone();
		m->set("albedoMap", Texture::get("Knight/" + diff));
		m->set("metallicMap", Texture::get("Knight/metallic_" + diff));
		m->set("roughnessMap", Texture::get("Knight/roughness_" + diff));
		materials.push_back(m);
	}

	// Load scene
	import_node(scene, scene->mRootNode, materials);
	return nullptr;
}
