#pragma once

#include <stdint.h>

#include "Core/System.h"
#include "Render/Shader/Material.h"
#include "Render/Texture/Texture.h"
#include "Render/CommandBuffer.h"

struct PostProcessingSystem
{
	PostProcessingSystem(void *system_dependency, Texture depth_texture, Texture color_texture, vec4 ss_viewport = ivec4(0,0,1,1), Material post_processing = Material::none);
	~PostProcessingSystem();

	void *dependency;
	cmd_buffer_t cmd_buffer;
	vec4 ss_viewport;

	int enable;
	Texture depth, color;
	Material post_process;

	static const system_type_t type;

	// Serialization
	PostProcessingSystem(const SerializationContext &ctx);
	void save(SerializationContext &ctx) const;
};
