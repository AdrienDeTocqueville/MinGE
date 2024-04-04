#pragma once

#include "Core/AssetDatabase.h"

inline static GLint parse_internal_format(const char *format)
{
	if (format == NULL) return GL_RGB8;

	// sRGB
	else if (!strcmp(format, "sr8"))	return GL_SR8_EXT;
	else if (!strcmp(format, "srgb8"))	return GL_SRGB8;
	else if (!strcmp(format, "srgba8"))	return GL_SRGB8_ALPHA8;

	// 8bits - unsigned norm
	else if (!strcmp(format, "r8"))		return GL_R8;
	else if (!strcmp(format, "rgb8"))	return GL_RGB8;
	else if (!strcmp(format, "rgba8"))	return GL_RGBA8;

	// 16bits - signed float
	else if (!strcmp(format, "r16f"))	return GL_R16F;
	else if (!strcmp(format, "rgb16f"))	return GL_RGB16F;
	else if (!strcmp(format, "rgba16f"))	return GL_RGBA16F;

	// 32bits - signed float
	else if (!strcmp(format, "r32f"))	return GL_R32F;
	else if (!strcmp(format, "rgb32f"))	return GL_RGB32F;
	else if (!strcmp(format, "rgba32f"))	return GL_RGBA32F;

	// depth
	else if (!strcmp(format, "depth16"))	return GL_DEPTH_COMPONENT16;
	else if (!strcmp(format, "depth24"))	return GL_DEPTH_COMPONENT24;
	else if (!strcmp(format, "depth32"))	return GL_DEPTH_COMPONENT32F;

	// depth stencil
	else if (!strcmp(format, "d24s8"))	return GL_DEPTH24_STENCIL8;

	else Log::addf(Log::Warning, "Format is not supported: %s", format);
	return 0;
}

void texture_importer(import_context_t &ctx)
{
	int x, y, n;
	unsigned char *data = stbi_load(ctx.path, &x, &y, &n, 4);
	if (data == nullptr)
	{
		Log::addf(Log::Error, "Failed to read '%s'.", ctx.path);
		return;
	}

	Texture::settings_t settings;
	settings.width = x;
	settings.height = y;

	Texture texture = Texture::create(settings);
	texture.set_pixel_data((color32_t*)data);

	stbi_image_free(data);

	ctx.register_object(Texture::type, texture);
}

const asset_importer_t Texture::importer = []() {
	asset_importer_t i = {};

	i.import = texture_importer;
	return i;
}();
