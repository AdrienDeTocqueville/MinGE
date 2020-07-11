#pragma once

#include <stdint.h>

#include "Render/Mesh/Mesh.h"
#include "Render/RenderPass.h"

struct camera_data_t
{
	mat4 view_proj;
	vec3 position;
	ivec2 resolution;
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
	~cmd_buffer_t() { free(buffer); }

	void setup_camera(camera_data_t *camera);
	void set_framebuffer(uint32_t fbo, vec4 color, bool clear_depth);
	void draw_batch(submesh_data_t *submeshes, mat4 *matrices, uint32_t *sorted_indices,
			RenderPass::Type pass, uint32_t count);
	void fullscreen_pass(uint32_t fbo, ivec4 viewport, uint32_t material);

	// Should only be called from render thread
	void flush();

	uint8_t *buffer;
	uint32_t size, capacity;

private:
	enum Type { DrawBatch, SetFramebuffer, SetupCamera, FullscreenPass };

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

	struct set_framebuffer_t
	{
		uint32_t fbo;
		bool clear_depth;
		vec4 clear_color;
	};

	struct fullscreen_pass_t
	{
		uint32_t fbo;
		uint32_t material;
		ivec4 viewport;
	};
};
