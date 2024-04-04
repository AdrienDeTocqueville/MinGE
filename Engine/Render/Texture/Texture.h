#pragma once

#include "Core/AssetDatabase.h"

#include "Math/glm.h"
#include "Structures/ArrayList.h"

#include <Render/GLDriver.h>
#include <GL/glew.h>

struct color32_t
{
	uint8_t r, g, b, a;
};

struct Texture: public UID32
{
	struct texture_t : object_data_t
	{
		uint32_t handle;
		uvec2 size;
		color32_t *pixels;
	};

	struct settings_t
	{
		int width, height;
		GL::WrapMode wrap_mode_s;
		GL::WrapMode wrap_mode_t;
		GL::FilterMode filter_mode;
		int format;
		bool generate_mips;
		const char *name;

		settings_t()
		{
			width = height = 1;
			wrap_mode_s = GL::Repeat;
			wrap_mode_t = GL::Repeat;
			filter_mode = GL::Linear;
			format = GL_RGB8;
			generate_mips = false;
			name = nullptr;
		}
	};

	Texture() {}
	Texture(UID32 uid): UID32(uid) {}

	void destroy();
	bool is_valid() const { return valid() && textures[id()].valid(gen()); }

	uint32_t handle() const { return textures[id()].handle; }
	uvec2 size() const { return textures[id()].size; }

	void set_pixel_data(color32_t *data);

	static Texture create(const settings_t &settings);
	static Texture get(uint32_t i) { return UID32(i, textures[i].gen); }

	static array_list_t<texture_t> textures;

	static const struct object_type_t type;
	static const struct asset_importer_t importer;
};
