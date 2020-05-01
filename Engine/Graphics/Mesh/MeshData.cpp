#include "Graphics/Mesh/Mesh.h"

void mesh_data_t::init(uint32_t _vertex_count, uint32_t _index_count, mesh_data_t::flags_t _flags)
{
	vertex_count = _vertex_count;
	index_count = _index_count;

	// TODO: make only one allocation
	points	= _flags & mesh_data_t::flags_t::POINTS	? new vec3[vertex_count] : NULL;
	normals	= _flags & mesh_data_t::flags_t::NORMALS? new vec3[vertex_count] : NULL;
	uvs	= _flags & mesh_data_t::flags_t::UVs	? new vec2[vertex_count] : NULL;
	bones	= _flags & mesh_data_t::flags_t::BONES	? new mesh_data_t::bone_weight_t[vertex_count] : NULL;

	indices = new uint16_t[index_count];
}

void mesh_data_t::free()
{
	delete[] points;
	delete[] normals;
	delete[] uvs;
	delete[] bones;

	delete[] indices;
}

uint32_t mesh_data_t::stride() const
{
	uint32_t stride = 0;
	if (points)	stride += sizeof(*points);
	if (normals)	stride += sizeof(*normals);
	if (uvs)	stride += sizeof(*uvs);
	if (bones)	stride += sizeof(*bones);

	return stride;
}
