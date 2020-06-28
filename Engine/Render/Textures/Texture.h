#pragma once

#include "Core/Asset.h"
#include "Core/UID.h"

#include "Render/GLDriver.h"
#include "Structures/MultiArray.h"

struct Texture: public UID32
{
	struct texture_t
	{
		uint32_t handle;
		uvec2 size;
	};

	Texture() {}

	void destroy();
	uint32_t handle() const { return id() ? textures.get<0>(id())->handle : 0; }
	uvec2 size() const { return textures.get<0>(id())->size; }
	const char *uri() const { return *textures.get<1>(id()); }
	bool is_valid() const { return id() && *textures.get<2>(id()) == gen(); }

	static Texture load(const char *URI);
	static Texture get(uint32_t i);
	static Texture find_by_handle(uint32_t handle);
	static void clear();

	static const Texture none;
	static multi_array_t<texture_t, char*, uint8_t> textures;

	static const asset_type_t type;

private:
	Texture(uint32_t i, uint32_t gen): UID32(i, gen) {}
};
