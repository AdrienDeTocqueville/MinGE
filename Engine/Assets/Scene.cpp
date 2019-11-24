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
	Transform *curr;

	if (node->mNumMeshes)
		curr = parent;
	else
	{
		std::string name = node->mName.C_Str();
		mat4 matrix = assimp_glm(node->mTransformation);

		glm::vec3 pos, scale, skew;
		glm::quat rotation;
		glm::vec4 perspective;
		glm::decompose(matrix, scale, rotation, pos, skew, perspective);

		swap_yz(pos, rotation);

		Entity *e = Entity::create(name, false, pos, rotation, scale);
		curr = e->find<Transform>();
		curr->setParent(parent);

		skeleton.bone_index[name] = skeleton.offsets.size();
		skeleton.offsets.emplace_back(1.0f);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		import_node(node->mChildren[i], curr, skeleton);
}

static MeshRef import_mesh(const aiScene *scene, Transform *root, Skeleton &skeleton, std::vector<unsigned> &materials)
{
	// 1 - Determine what data is needed and sort by materials
	bool has_pos = true, has_normal = true, has_uv = true;
	int vertex_count = 0, index_count = 0, bone_count = 0;

	std::vector<Submesh> submeshes;
	submeshes.reserve(scene->mNumMeshes);

	std::vector<aiMesh*> meshes;
	for (unsigned i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh *submesh = scene->mMeshes[i];
		meshes.push_back(submesh);

		if (!submesh->HasPositions()) has_pos = false;
		if (!submesh->HasNormals()) has_normal = false;
		if (!submesh->HasTextureCoords(0)) has_uv = false;

		vertex_count += submesh->mNumVertices;
		index_count += submesh->mNumFaces * 3;
		bone_count += submesh->mNumBones;
	}

	std::sort(meshes.begin(), meshes.end(), [] (aiMesh *&a, aiMesh *&b) {
		return a->mMaterialIndex < b->mMaterialIndex;
	});

	// 2 - Allocate necessary space
	MeshData::Flags flags = MeshData::Empty;
	if (has_pos)	flags = (MeshData::Flags)(flags | MeshData::Points);
	if (has_normal)	flags = (MeshData::Flags)(flags | MeshData::Normals);
	if (has_uv)	flags = (MeshData::Flags)(flags | MeshData::UVs);
	if (bone_count)	flags = (MeshData::Flags)(flags | MeshData::Bones);

	MeshData data(vertex_count, index_count, flags);

	if (flags & MeshData::Bones)
	{
		memset(data.bones, 0, sizeof(*data.bones) * data.vertex_count);
		for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++)
			import_node(scene->mRootNode->mChildren[i], root, skeleton);
	}

	// 3 - Read data
	unsigned prev_mat = scene->mNumMaterials;
	vertex_count = index_count = bone_count = 0;
	for (aiMesh *submesh : meshes)
	{
		if (submesh->mMaterialIndex == prev_mat)
			submeshes.back().count += submesh->mNumFaces * 3;
		else
		{
			prev_mat = submesh->mMaterialIndex;
			submeshes.emplace_back(GL_TRIANGLES, submesh->mNumFaces * 3, index_count);
			materials.push_back(prev_mat);
		}

		// Read vertices
		unsigned numVertices = submesh->mNumVertices;
		for (unsigned i = 0; i < numVertices; i++)
		{
			size_t v = vertex_count + i;

			if (flags & MeshData::Points)
				data.points[v] = vec3(
					submesh->mVertices[i].x,
					submesh->mVertices[i].y,
					submesh->mVertices[i].z
				);

			if (flags & MeshData::Normals)
				data.normals[v] = vec3(
					submesh->mNormals[i].x,
					submesh->mNormals[i].y,
					submesh->mNormals[i].z
				);

			if (flags & MeshData::UVs)
				data.uvs[v] = vec2(
					submesh->mTextureCoords[0][i].x,
					submesh->mTextureCoords[0][i].y
				);
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
		if (flags & MeshData::Bones)
		{
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
		}

		vertex_count += numVertices;
	}

	return MeshRef(new Mesh(std::move(data), submeshes));
}

static std::vector<MaterialRef> import_materials(const aiScene *scene, const std::vector<unsigned> &material_indices, bool skinned, const std::string &base_path)
{
	MaterialRef base = Material::create(skinned ? "skinned" : "object");

	std::vector<MaterialRef> materials;
	for (unsigned i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial *mat = scene->mMaterials[i];

		aiString path;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		std::string texture_path(path.C_Str());

		MaterialRef m = base->clone();
		//m->set("albedoMap", Texture::get(base_path + texture_path));
		//m->set("metallicMap", Texture::get("Knight/metallic_" + diff));
		//m->set("roughnessMap", Texture::get("Knight/roughness_" + diff));
		materials.push_back(m);
	}

	std::vector<MaterialRef> mesh_materials;
	for (unsigned index : material_indices)
		mesh_materials.push_back(materials[index]);

	return mesh_materials;
}

// Ignores scaling
static std::vector<AnimationRef> import_animations(const aiScene *scene, const Skeleton &skeleton)
{
	std::vector<AnimationRef> animations; animations.reserve(scene->mNumAnimations);

	for (unsigned i = 0; i < scene->mNumAnimations; i++)
	{
		aiAnimation *aiAnim = scene->mAnimations[i];
		float rate = 1.0f / aiAnim->mTicksPerSecond;

		Animation *anim = new Animation(aiAnim->mName.C_Str(), aiAnim->mDuration * rate);

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
					node->mPositionKeys[key].mTime * rate,
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

static inline void dump_meshes(const aiScene *scene)
{
	for (size_t i(0); i < scene->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[i];
		printf(" - %s (%d faces) (%d bones) (material %d)\n", mesh->mName.C_Str(), mesh->mNumFaces, mesh->mNumBones, mesh->mMaterialIndex);
	}
}

static inline void dump_animations(const aiScene *scene)
{
	for (size_t i(0); i < scene->mNumAnimations; i++)
	{
		aiAnimation *anim = scene->mAnimations[i];
		printf(" - %s\n", anim->mName.C_Str());
	}
}

static void dump(const aiScene *scene)
{
	printf("> Scene\n - meshes: %d\n - materials: %d\n - textures: %d\n", scene->mNumMeshes, scene->mNumMaterials, scene->mNumTextures);
	printf(" - animations: %d\n - lights: %d\n - cameras: %d\n\n", scene->mNumAnimations, scene->mNumLights, scene->mNumCameras);

	printf("> Root node\n");
	dump_node(scene->mRootNode);
	printf("\n\n");

	printf("> Meshes\n");
	dump_meshes(scene);
	printf("\n\n");

	if (scene->HasAnimations())
	{
		printf("> Animations\n");
		dump_animations(scene);
		printf("\n\n");
	}
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
	if (!scene || !scene->mRootNode)
	{
		Error::add(FILE_NOT_FOUND, "Scene::import() -> " + path);
		return nullptr;
	}
	if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
		Error::add(WARNING, "Scene::import() -> Incomplete scene :" + path);

#ifdef DEBUG
	dump(scene);
#endif

	//Entity *e = Entity::create(base, true);
	Entity *e = Entity::create(base, false);

	Skeleton skeleton;
	std::vector<unsigned> material_indices;

	auto mesh = import_mesh(scene, e->find<Transform>(), skeleton, material_indices);
	auto materials = import_materials(scene, material_indices, skeleton.offsets.size(), base + '/');
	e->insert<Graphic>(mesh, materials);

	if (skeleton.offsets.size())
		e->insert<Animator>(skeleton);

	return e;
}

AnimationRef Scene::import_animation(const std::string &file, const Skeleton &skeleton)
{
	std::string path = "Resources/" + file;
	std::string base = file.substr(0, file.find_last_of("/\\"));

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, 0);

	// check for errors
	if (!scene || !scene->mRootNode)
	{
		Error::add(FILE_NOT_FOUND, "Scene::import() -> " + path);
		return nullptr;
	}

	if (!scene->HasAnimations())
	{
		Error::add(WARNING, "Scene::import() -> Scene has no animations :" + path);
		return nullptr;
	}

#ifdef DEBUG
	dump(scene);
#endif

	return import_animations(scene, skeleton)[0];
}
