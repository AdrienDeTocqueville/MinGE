#pragma once

#include <cstdint>
#include "Math/glm.h"

struct Mesh
{
	Mesh(): index(0) {}
	inline uint32_t id() { return index; }

	static const Mesh none;
	static Mesh import(const char *URI);
	static void clear();

private:
	Mesh(uint32_t i): index(i) {}
	uint32_t index;
};


struct submeshes_t
{
	uint32_t first, last;
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

bool generate_mesh(const struct uri_t &uri, mesh_data_t &data);
