#include "Assets/Scene.h"

#include "MinGE.h"
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

static unsigned argmin_weight(vec4 weights)
{
	unsigned argmin = 0;
	float min_val = 1.0f; // Weights should be between 0 and 1
	for (unsigned k(0); k < 4; k++)
	{
		if (weights[k] == 0.0f)
			return k;
		if (weights[k] < min_val)
		{
			min_val = weights[k];
			argmin = k;
		}
		k++;
	}
	return argmin;
}

static mat4 assimp_glm(const aiMatrix4x4 &from)
{
	mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;

	return to;
}

static vec3 assimp_glm(const aiVector3D &from)
{
	return vec3(from.x, -from.z, from.y);
}

static quat assimp_glm(const aiQuaternion &from)
{
	return quat(from.w, from.x, -from.z, from.y);
}

static void swap_yz(mat4 &mat)
{
	mat4 t(1.0f);
	t[1][1] = 0.0f;
	t[2][2] = 0.0f;
	t[2][1] = -1.0f;
	t[1][2] = 1.0f;

	mat = t * mat;
}

static void swap_yz(vec3 &pos, quat &rot)
{
	// pos
	float temp = pos.z;
	pos.z = pos.y;
	pos.y = -temp;

	// rot
	temp = rot.z;
	rot.z = rot.y;
	rot.y = -temp;
}


static void import_node(aiNode *node, Transform *parent, Skeleton &skeleton)
{
	std::string name = node->mName.C_Str();
	mat4 matrix = assimp_glm(node->mTransformation);

	glm::vec3 pos, scale, skew;
	glm::quat rotation;
	glm::vec4 perspective;
	glm::decompose(matrix, scale, rotation, pos, skew, perspective);

	swap_yz(pos, rotation);

	Entity *e = Entity::create(name, false, pos, rotation, scale);
	Transform *curr = e->find<Transform>();
	curr->setParent(parent);

	skeleton.bone_index[name] = skeleton.nodes.size();
	skeleton.nodes.push_back(curr);
	skeleton.offsets.emplace_back(1.0f);

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		import_node(node->mChildren[i], curr, skeleton);
}

static inline Skeleton import_skeleton(const aiScene *scene, Entity *e)
{
	Skeleton skeleton;

	import_node(scene->mRootNode, e->find<Transform>(), skeleton);
	return skeleton;
}

static MeshRef import_mesh(const aiScene *scene, Skeleton &skeleton)
{
	bool has_bones = true;
	int vertex_count = 0, index_count = 0, bone_count = 0;

	std::vector<Submesh> submeshes;
	submeshes.reserve(scene->mNumMeshes);

	for (unsigned i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh *submesh = scene->mMeshes[i];

		unsigned numVertices = submesh->mNumVertices;
		unsigned numIndices = submesh->mNumFaces * 3;
		unsigned numBones = submesh->mNumBones;

		submeshes.emplace_back(GL_TRIANGLES, numIndices, index_count);
		if (numBones == 0)
			has_bones = false;

		vertex_count += numVertices;
		index_count += numIndices;
		bone_count += numBones;
	}

	MeshData::Flags flags = has_bones ? MeshData::Full : MeshData::Basic;
	MeshData data(vertex_count, index_count, flags);

	if (flags & MeshData::Bones)
		memset(data.bones, 0, sizeof(*data.bones) * data.vertex_count);

	// Read data
	vertex_count = index_count = bone_count = 0;
	for (unsigned sub = 0; sub < scene->mNumMeshes; sub++)
	{
		aiMesh *submesh = scene->mMeshes[sub];

		// Read vertices
		unsigned numVertices = submesh->mNumVertices;
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
		unsigned numFaces = submesh->mNumFaces;
		for (unsigned i = 0; i < numFaces; i++)
		{
			auto *face = submesh->mFaces[i].mIndices;
			for (unsigned j = 0; j < 3; j++)
				data.indices[index_count++] = vertex_count + face[j];
		}

		// Read bones
		unsigned numBones = submesh->mNumBones;
		for (unsigned i = 0; i < numBones; i++)
		{
			aiBone *bone = submesh->mBones[i];
			std::string name(bone->mName.C_Str());

			auto it = skeleton.bone_index.find(name);
			if (it == skeleton.bone_index.end())
				Error::add(USER_ERROR, "import_mesh(): invalid data");

			unsigned bone_index = it->second;

			skeleton.offsets[bone_index] = assimp_glm(bone->mOffsetMatrix);
			swap_yz(skeleton.offsets[bone_index]);

			for (unsigned j = 0; j < bone->mNumWeights; j++)
			{
				unsigned index = vertex_count + bone->mWeights[j].mVertexId;
				unsigned k = argmin_weight(data.bones[index].weights);
				data.bones[index].bones[k] = bone_index;
				data.bones[index].weights[k] = bone->mWeights[j].mWeight;
			}
		}

		vertex_count += numVertices;
	}

	return MeshRef(new Mesh(std::move(data), submeshes));
}

static std::vector<MaterialRef> import_materials(const aiScene *scene, const std::string &base_path)
{
	std::vector<MaterialRef> materials;
	for (unsigned i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial *mat = scene->mMaterials[i];

		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		std::string texture_path(path.C_Str());

		MaterialRef m = Material::create("skinned");
		m->set("albedoMap", Texture::get(base_path + texture_path));
		//m->set("metallicMap", Texture::get("Knight/metallic_" + diff));
		//m->set("roughnessMap", Texture::get("Knight/roughness_" + diff));
		materials.push_back(m);
	}

	std::vector<MaterialRef> mesh_materials;
	for (unsigned i = 0; i < scene->mNumMeshes; i++)
		mesh_materials.push_back(materials[scene->mMeshes[i]->mMaterialIndex]);

	return mesh_materials;
}

// Ignores scaling
static std::vector<AnimationRef> import_animations(const aiScene *scene, Skeleton &skeleton)
{
	std::vector<AnimationRef> animations; animations.reserve(scene->mNumAnimations);

	for (unsigned i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation *aiAnim = scene->mAnimations[i];
		Animation *anim = new Animation(aiAnim->mName.C_Str(), aiAnim->mDuration / aiAnim->mTicksPerSecond, true);

		anim->channels.reserve(aiAnim->mNumChannels);
		for (unsigned j = 0; j < aiAnim->mNumChannels; j++)
		{
			aiNodeAnim *node = aiAnim->mChannels[j];

			if (node->mNumPositionKeys != node->mNumRotationKeys)
				Error::add(USER_ERROR, "import_animations(): unsupported channel");

			auto it = skeleton.bone_index.find(node->mNodeName.C_Str());
			if (it == skeleton.bone_index.end())
				Error::add(USER_ERROR, "import_animations(): invalid data");

			unsigned key_count = node->mNumPositionKeys;
			Animation::Track track(it->second, key_count);

			for (unsigned key = 0; key < key_count; key++)
			{
				if (node->mPositionKeys[key].mTime != node->mRotationKeys[key].mTime)
					Error::add(USER_ERROR, "import_animations(): unsupported key");

				track.keys.emplace_back(
					node->mPositionKeys[key].mTime,
					assimp_glm(node->mPositionKeys[key].mValue),
					assimp_glm(node->mRotationKeys[key].mValue)
				);
			}

			anim->channels.emplace_back(std::move(track));
		}

		animations.push_back(AnimationRef(anim));
	}
	return animations;
}

static void dump_node(aiNode *node, int recur = 0)
{
	for (int i(0); i < recur; i++)
		printf("  ");
	printf("%s (%d children) (%d meshes)\n", node->mName.C_Str(), node->mNumChildren, node->mNumMeshes);

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		dump_node(node->mChildren[i], recur + 1);
}

static inline void dump_mesh(aiMesh *mesh)
{
	printf(" - %s (%d faces) (%d bones)\n", mesh->mName.C_Str(), mesh->mNumFaces, mesh->mNumBones);
}

static void dump(const aiScene *scene)
{
	printf("> Scene\n - meshes: %d\n - materials: %d\n - textures: %d\n", scene->mNumMeshes, scene->mNumMaterials, scene->mNumTextures);
	printf(" - animations: %d\n - lights: %d\n - cameras: %d\n\n", scene->mNumAnimations, scene->mNumLights, scene->mNumCameras);

	printf("> Root node\n");
	dump_node(scene->mRootNode);
	printf("\n\n");

	printf("> Meshes\n");
	for (int i(0); i < scene->mNumMeshes; i++)
		dump_mesh(scene->mMeshes[i]);
	printf("\n\n");
}

Entity *Scene::import(const std::string &file)
{
	std::string path = "Resources/" + file;
	std::string base = file.substr(0, file.find_last_of("/\\"));

	// read file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
			aiProcess_GenNormals);

	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		Error::add(FILE_NOT_FOUND, "Scene::import() -> " + path);
		return nullptr;
	}

#ifdef DEBUG
	dump(scene);
#endif

	//Entity *e = Entity::create(base, true);
	//Entity *e = Entity::create(base, false);
	Entity *e = Entity::create(base, false, vec3(0.0f), vec3(0,0,3.14f), vec3(0.4f));

	auto skeleton = import_skeleton(scene, e);
	auto mesh = import_mesh(scene, skeleton);
	auto materials = import_materials(scene, base + '/');
	auto animations = import_animations(scene, skeleton);

	e->insert<Graphic>(mesh, materials);
	e->insert<Animator>(skeleton, animations);
	return e;
}
