#include "Renderer/Mesh.h"

#include "IO/URI.h"
#include "Math/glm.h"
#include "Utility/Error.h"

static mesh_data_t::flags_t parse_flags(const uri_t &uri)
{
	int flags = 0;
	if (uri.get_or_default("points", true))	flags |= mesh_data_t::POINTS;
	if (uri.get_or_default("normals", true))flags |= mesh_data_t::NORMALS;
	if (uri.get_or_default("uvs", true))	flags |= mesh_data_t::UVs;

	return (mesh_data_t::flags_t)flags;
}

bool generate_mesh(const uri_t &uri, mesh_data_t &data)
{
	mesh_data_t::flags_t flags = parse_flags(uri);

	if (uri.path == "mesh/cube")
	{
		vec3 e(uri.get_or_default("x", 0.5f), uri.get_or_default("y", 0.5f), uri.get_or_default("z", 0.5f));
		data.init(4 * 6, 6 * 6, flags);

		if (flags & mesh_data_t::POINTS)
		{
			size_t i(0);
			// Front Face
			data.points[i++] = vec3( e.x, -e.y, -e.z);
			data.points[i++] = vec3( e.x,  e.y, -e.z);
			data.points[i++] = vec3( e.x,  e.y,  e.z);
			data.points[i++] = vec3( e.x, -e.y,  e.z);

			// Right Face
			data.points[i++] = vec3(-e.x, -e.y, -e.z);
			data.points[i++] = vec3( e.x, -e.y, -e.z);
			data.points[i++] = vec3( e.x, -e.y,  e.z);
			data.points[i++] = vec3(-e.x, -e.y,  e.z);

			// Back Face
			data.points[i++] = vec3(-e.x,  e.y, -e.z);
			data.points[i++] = vec3(-e.x, -e.y, -e.z);
			data.points[i++] = vec3(-e.x, -e.y,  e.z);
			data.points[i++] = vec3(-e.x,  e.y,  e.z);

			// Left Face
			data.points[i++] = vec3( e.x,  e.y, -e.z);
			data.points[i++] = vec3(-e.x,  e.y, -e.z);
			data.points[i++] = vec3(-e.x,  e.y,  e.z);
			data.points[i++] = vec3( e.x,  e.y,  e.z);

			// Top Face
			data.points[i++] = vec3(-e.x, -e.y,  e.z);
			data.points[i++] = vec3( e.x, -e.y,  e.z);
			data.points[i++] = vec3( e.x,  e.y,  e.z);
			data.points[i++] = vec3(-e.x,  e.y,  e.z);

			// Bottom Face
			data.points[i++] = vec3( e.x, -e.y, -e.z);
			data.points[i++] = vec3(-e.x, -e.y, -e.z);
			data.points[i++] = vec3(-e.x,  e.y, -e.z);
			data.points[i++] = vec3( e.x,  e.y, -e.z);
		}
		if (flags & mesh_data_t::NORMALS)
		{
			size_t i(0);
			// Front Face
			data.normals[i++] = vec3( 1,  0,  0);
			data.normals[i++] = vec3( 1,  0,  0);
			data.normals[i++] = vec3( 1,  0,  0);
			data.normals[i++] = vec3( 1,  0,  0);

			// Right Face
			data.normals[i++] = vec3( 0, -1,  0);
			data.normals[i++] = vec3( 0, -1,  0);
			data.normals[i++] = vec3( 0, -1,  0);
			data.normals[i++] = vec3( 0, -1,  0);

			// Back Face
			data.normals[i++] = vec3(-1,  0,  0);
			data.normals[i++] = vec3(-1,  0,  0);
			data.normals[i++] = vec3(-1,  0,  0);
			data.normals[i++] = vec3(-1,  0,  0);

			// Left Face
			data.normals[i++] = vec3( 0,  1,  0);
			data.normals[i++] = vec3( 0,  1,  0);
			data.normals[i++] = vec3( 0,  1,  0);
			data.normals[i++] = vec3( 0,  1,  0);

			// Top Face
			data.normals[i++] = vec3( 0,  0,  1);
			data.normals[i++] = vec3( 0,  0,  1);
			data.normals[i++] = vec3( 0,  0,  1);
			data.normals[i++] = vec3( 0,  0,  1);

			// Bottom Face
			data.normals[i++] = vec3( 0,  0, -1);
			data.normals[i++] = vec3( 0,  0, -1);
			data.normals[i++] = vec3( 0,  0, -1);
			data.normals[i++] = vec3( 0,  0, -1);
		}
		if (flags & mesh_data_t::UVs)
		{
			size_t i(0);
			// Front Face
			data.uvs[i++] = vec2(0, 0);
			data.uvs[i++] = vec2(0, 1);
			data.uvs[i++] = vec2(1, 1);
			data.uvs[i++] = vec2(1, 0);

			// Right Face
			data.uvs[i++] = vec2(0, 0);
			data.uvs[i++] = vec2(0, 1);
			data.uvs[i++] = vec2(1, 1);
			data.uvs[i++] = vec2(1, 0);

			// Back Face
			data.uvs[i++] = vec2(0, 0);
			data.uvs[i++] = vec2(0, 1);
			data.uvs[i++] = vec2(1, 1);
			data.uvs[i++] = vec2(1, 0);

			// Left Face
			data.uvs[i++] = vec2(0, 0);
			data.uvs[i++] = vec2(0, 1);
			data.uvs[i++] = vec2(1, 1);
			data.uvs[i++] = vec2(1, 0);

			// Top Face
			data.uvs[i++] = vec2(0, 0);
			data.uvs[i++] = vec2(0, 1);
			data.uvs[i++] = vec2(1, 1);
			data.uvs[i++] = vec2(1, 0);

			// Bottom Face
			data.uvs[i++] = vec2(0, 0);
			data.uvs[i++] = vec2(0, 1);
			data.uvs[i++] = vec2(1, 1);
			data.uvs[i++] = vec2(1, 0);
		}

		static const uint16_t indices[] = {
			0,  1,  2,  0,  2,  3,
			4,  5,  6,  4,  6,  7,
			8,  9,  10, 8,  10, 11,
			12, 13, 14, 12, 14, 15,
			16, 17, 18, 16, 18, 19,
			20, 21, 22, 20, 22, 23,
		};
		memcpy(data.indices, indices, sizeof(indices));
	}
	else if (uri.path == "mesh/quad")
	{
		vec2 e(uri.get_or_default("x", 0.5f), uri.get_or_default("y", 0.5f));
		uvec2 subdiv(uri.get_or_default("subdiv-x", 2), uri.get_or_default("subdiv-y", 2));
		vec2 tiling(uri.get_or_default("tiling-x", 1), uri.get_or_default("tiling-y", 1));

		data.init(subdiv.x * subdiv.y, (subdiv.x - 1) * (subdiv.y - 1) * 6, flags);

		size_t v(0);
		for (unsigned i = 0; i < subdiv.y; i++)
		{
			for (unsigned j = 0; j < subdiv.x; j++)
			{
				vec2 uv(
					float(j) / (subdiv.x - 1),
					float(i) / (subdiv.y - 1)
				);

				if (flags & mesh_data_t::POINTS)
					data.points[v] = vec3(
						e.x * (uv.x * 2.0f - 1.0f),
						e.y * (uv.y * 2.0f - 1.0f),
						0.0f
					);
				if (flags & mesh_data_t::NORMALS)
					data.normals[v] = vec3(0, 0, 1);
				if (flags & mesh_data_t::UVs)
					data.uvs[v] = uv * tiling;
				v++;
			}
		}

		v = 0;
		for (unsigned i = 0; i < subdiv.y - 1; i++)
		{
			for (unsigned j = 0; j < subdiv.x - 1; j++)
			{
				data.indices[v++] = j + 0 + subdiv.x * (i + 0);
				data.indices[v++] = j + 1 + subdiv.x * (i + 0);
				data.indices[v++] = j + 0 + subdiv.x * (i + 1);

				data.indices[v++] = j + 1 + subdiv.x * (i + 0);
				data.indices[v++] = j + 1 + subdiv.x * (i + 1);
				data.indices[v++] = j + 0 + subdiv.x * (i + 1);
			}
		}
	}
	else if (uri.path == "mesh/sphere")
	{
		float radius(uri.get_or_default("radius", 0.5f));
		unsigned slices(uri.get_or_default("slices", 20));
		unsigned stacks(uri.get_or_default("stacks", 10));

		const float iStacks = 1.0f/(float)(stacks-1);
		const float iSlices = 1.0f/(float)(slices-1);

		float slice_step = 2 * PI * iSlices;
		float stack_step = PI * iStacks;
		float slice_angle, stack_angle;

		data.init((slices+1) * (stacks+1), 6 * slices * (stacks-1), flags);

		// Generate vertices
		size_t v(0);
		for (unsigned i = 0; i <= stacks; ++i)
		{
			stack_angle = 0.5f * PI - i * stack_step;
			float xy = cosf(stack_angle);
			float z = sinf(stack_angle);

			for (unsigned j = 0; j <= slices; ++j)
			{
				slice_angle = j * slice_step;

				vec3 normal(xy * cosf(slice_angle), xy * sinf(slice_angle), z);

				if (flags & mesh_data_t::POINTS)
					data.points[v] = normal * radius;
				if (flags & mesh_data_t::NORMALS)
					data.normals[v] = normal;
				if (flags & mesh_data_t::UVs)
					data.uvs[v] = vec2(j*iSlices, i*iStacks);
				v++;
			}
		}

		// Generate indices
		v = 0;
		for (unsigned i = 0; i < stacks; ++i)
		{
			uint16_t k1 = i * (slices + 1);
			uint16_t k2 = k1 + slices + 1;

			for (unsigned j = 0; j < slices; ++j, ++k1, ++k2)
			{
				if (i != 0) {
					data.indices[v++] = k1;
					data.indices[v++] = k2;
					data.indices[v++] = k1 + 1;
				}

				if (i != (stacks-1)) {
					data.indices[v++] = k1 + 1;
					data.indices[v++] = k2;
					data.indices[v++] = k2 + 1;
				}
			}
		}
	}
	else
	{
		Error::add(Error::USER, "Invalid asset type");
		return false;
	}
	return true;
}
