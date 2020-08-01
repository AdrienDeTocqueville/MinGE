#pragma once

#include <stdint.h>

#include "Render/Mesh/Mesh.h"
#include "Render/RenderPass.h"

struct submesh_data_t
{
	submesh_t submesh;
	uint32_t material;
	uint32_t renderer;
};

struct cmd_buffer_t
{
	static void init();
	static void destroy();

	cmd_buffer_t(): size(0), capacity(32) { buffer = (uint8_t*)malloc(capacity); }
	~cmd_buffer_t() { free(buffer); }

	void set_uniform_data(uint32_t buffer, void *data, uint32_t size);
	void set_storage_data(uint32_t buffer, void *data, uint32_t size);

	void setup_camera(ivec2 res, uint32_t buf, uint32_t offset, uint32_t size);
	void set_framebuffer(uint32_t fbo, vec4 color, bool clear_depth);

	void draw_batch(submesh_data_t *submeshes, uint32_t *sorted_indices,
			uint32_t per_object, RenderPass::Type pass, uint32_t count);
	void fullscreen_pass(uint32_t fbo, ivec4 viewport, uint32_t material);

	// Should only be called from render thread
	void flush();

	uint8_t *buffer;
	uint32_t size, capacity;

private:
	enum Type { SetUniformData, SetStorageData, DrawBatch, SetFramebuffer, SetupCamera, FullscreenPass };

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

	struct block_data_t
	{
		uint32_t buffer;
		uint32_t size;
		void *data;
	};

	struct draw_batch_t
	{
		submesh_data_t *submeshes;
		uint32_t *sorted_indices;
		uint32_t per_object;
		RenderPass::Type pass;
		uint32_t count;
	};

	struct set_framebuffer_t
	{
		uint32_t fbo;
		bool clear_depth;
		vec4 clear_color;
	};

	struct setup_camera_t
	{
		ivec2 res;
		uint32_t buf;
		uint32_t offset, size;
	};

	struct fullscreen_pass_t
	{
		uint32_t fbo;
		uint32_t material;
		ivec4 viewport;
	};
};
