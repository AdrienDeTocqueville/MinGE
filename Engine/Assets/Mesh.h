#pragma once

#include <memory>

#include "Utility/helpers.h"
#include "Utility/Accel/AABB.h"

class Transform;

struct Submesh
{
	Submesh(uint32_t _mode, uint32_t _count, uint32_t _first_index = 0):
		mode(_mode), count(_count), offset(_first_index * sizeof(uint16_t)) { }

	uint32_t mode;
	uint32_t count;
	uint32_t offset;
};

struct BoneWeight
{
	uvec4 bones;
	vec4  weights;
};

struct MeshData
{
	enum Flags
	{
		Points = 1,
		Normals = 2,
		UVs = 4,
		Bones = 8,

		Empty = 0,
		Basic = (Points | Normals | UVs),
		Full = (Points | Normals | UVs | Bones),
	};

	MeshData(uint32_t _vertex_count, uint32_t _index_count, MeshData::Flags _flags = Basic);
	MeshData(MeshData &&data);
	~MeshData();

	uint32_t stride() const;

	MeshData(const MeshData&) = delete;
	void operator=(const MeshData&) = delete;

	uint32_t vertex_count;
	uint32_t index_count;

	vec3 *points;
	vec3 *normals;
	vec2 *uvs;
	BoneWeight *bones;

	uint16_t *indices;
};

typedef std::shared_ptr<class Mesh> MeshRef;

class Mesh
{
public:
	Mesh(MeshData &&_data, const std::vector<Submesh> &submeshes);
	~Mesh();

	/// Getters
	AABB getAABB() const
	{ return aabb; }
	unsigned getVAO() const
	{ return vao; }
	const std::vector<Submesh> &getSubmeshes() const
	{ return submeshes; }

	/// Methods (static)
	static MeshRef createCube(MeshData::Flags flags = MeshData::Basic, vec3 halfExtent = vec3(0.5f));
	static MeshRef createQuad(MeshData::Flags flags = MeshData::Basic, vec2 halfExtent = vec2(0.5f), uvec2 subdiv = uvec2(2), vec2 tiling = vec2(1));
	static MeshRef createSphere(MeshData::Flags flags = MeshData::Basic, float radius = 0.5f, unsigned slices = 20, unsigned stacks = 10);
	static MeshRef createCylinder(MeshData::Flags flags = MeshData::Basic, float _base = 0.5f, float _top = 0.5f, float _height = 1.0f, unsigned _slices = 20);

private:
	void loadBuffers();

	std::vector<Submesh> submeshes;
	MeshData data;

	unsigned vao, vbo, ebo;
	AABB aabb;
};
