#pragma once

#include "Core/AssetDatabase.h"

#include "Math/glm.h"
#include "Structures/Bounds.h"
#include "Structures/ArrayList.h"

struct submesh_t
{
	uint32_t mode, count, offset;
	uint32_t vao;
};

struct mesh_data_t
{
	enum flags_t
	{
		POINTS  = 1 << 0,
		NORMALS = 1 << 1,
		UVs     = 1 << 2,
		BONES   = 1 << 3,

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
	struct submeshes_t
	{
		uint32_t first, count;
		uint32_t vbo, ebo;
	};

	struct mesh_t : object_data_t
	{
		submeshes_t submeshes;
		mesh_data_t data;
		AABB aabb;
	};

	Mesh() {}
	Mesh(UID32 uid): UID32(uid) {}

	void destroy();
	bool is_valid() const { return valid() && meshes[id()].valid(gen()); }

	submeshes_t submesh_list() const { return meshes[id()].submeshes; }
	mesh_data_t data() const { return meshes[id()].data; }
	AABB aabb() const { return meshes[id()].aabb; }

	void set_mesh_data(mesh_data_t &&data, const submesh_t *submeshes, uint32_t submesh_count);

	static Mesh create(const char *name);
	static Mesh get(uint32_t i) { return UID32(i, meshes[i].gen); }

	static array_list_t<mesh_t> meshes;
	static array_list_t<submesh_t> submeshes;

	static const object_type_t type;
};
