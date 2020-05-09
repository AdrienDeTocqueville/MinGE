#pragma once

#include <stdint.h>

#include "Graphics/Mesh/Mesh.h"
#include "Graphics/RenderPass.h"

struct camera_data_t
{
	ivec4 viewport;
	vec4 clear_color;

	mat4 view_proj;
	vec3 position;

	uint32_t clear_flags;
};

struct submesh_data_t
{
	submesh_t submesh;
	uint32_t material;
	uint32_t renderer;
};

struct cmd_buffer_t
{
	cmd_buffer_t(): size(0), capacity(32) { buffer = (uint8_t*)malloc(capacity); }

	void draw_batch(submesh_data_t *submeshes, mat4 *matrices, uint32_t *sorted_indices,
			RenderPass::Type pass, uint32_t count);
	void setup_camera(camera_data_t *camera);

	void flush();

	uint8_t *buffer;
	uint32_t size, capacity;

private:
	enum Type { DrawBatch, SetupCamera };

	template <typename T>
	void store(Type type, T data)
	{
		uint32_t new_size = size + sizeof(Type) + sizeof(T);
		if (new_size > capacity)
		{
			capacity *= 2;
			buffer = (uint8_t*)realloc(buffer, capacity);
		}
		*(Type*)(buffer + size) = type;
		memcpy(buffer + size + sizeof(Type), &data, sizeof(T));
		size = new_size;
	}

	template <typename T>
	T &consume(uint32_t &pos)
	{
		uint8_t *data = buffer + pos;
		pos += sizeof(T);
		return *(T*)data;
	}

	struct draw_batch_t
	{
		submesh_data_t *submeshes;
		mat4 *matrices;
		uint32_t *sorted_indices;
		RenderPass::Type pass;
		uint32_t count;
	};
};
