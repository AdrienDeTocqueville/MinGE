#pragma once

#include <stdint.h>

#include "Core/System.h"
#include "Render/Textures/Texture.h"

struct PostProcessingSystem
{
	PostProcessingSystem(void *system_dependency, Texture color_texture, float exposure = 1.0, vec4 ss_viewport = ivec4(0,0,1,1));
	~PostProcessingSystem();

	void *dependency;
	uint32_t cmd_buffer;
	vec4 ss_viewport;

	int enable;
	Texture texture;
	float exposure;

	static const system_type_t type;

	// Serialization
	PostProcessingSystem(const SerializationContext &ctx);
	void save(SerializationContext &ctx) const;
};
