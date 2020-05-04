#pragma once

#include "Core/UID.h"
#include "Math/glm.h"

#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"

struct submeshes_t
{
	uint32_t first, count;
	uint32_t vbo, ebo;
};

struct submesh_t
{
	uint32_t mode, count, offset;
	uint32_t vao;
};

struct mesh_data_t
{
	enum flags_t
	{
		POINTS = 1,
		NORMALS = 2,
		UVs = 4,
		BONES = 8,

		EMPTY = 0,
		BASIC = (POINTS | NORMALS | UVs),
	};

	struct bone_weight_t
	{
		uvec4 bones;
		vec4  weights;
	};


	void init(uint32_t _vertex_count, uint32_t _index_count, flags_t _flags = flags_t::BASIC);
	void free();

	uint32_t stride() const;

	uint32_t vertex_count;
	uint32_t index_count;

	vec3 *points;
	vec3 *normals;
	vec2 *uvs;
	bone_weight_t *bones;

	uint16_t *indices;
};

struct Mesh: public UID32
{
	Mesh() {}

	bool is_valid() { return id() && *meshes.get<4>(id()) == gen(); }
	void destroy();

	static const Mesh none;
	static Mesh import(const char *URI);
	static void clear();

	// submeshes, mesh data, URI, AABB, generation
	static multi_array_t<submeshes_t, mesh_data_t, const char*, struct AABB, uint8_t> meshes;
	static array_list_t<submesh_t> submeshes;

private:
	Mesh(uint32_t i, uint32_t g): UID32(i, g) {}
};
