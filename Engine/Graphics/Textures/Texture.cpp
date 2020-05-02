#include "Graphics/Textures/Texture.h"
#include "Utility/Error.h"
#include "IO/URI.h"

#include <SFML/Graphics/Image.hpp>

struct texture_t
{
	uvec2 size;
};

const Texture Texture::none;
static std::vector<texture_t> textures;


bool Texture::is_valid()
{
	return glIsTexture((GLuint)id());
}

uvec2 Texture::size()
{
	return textures[id()].size;
}

void Texture::destroy()
{
	GLuint i = (GLuint)id();
	glCheck(glDeleteTextures(1, &i));
}


GLint parse_wrap_mode(const char *mode)
{
	if (mode)
	{
		if (strcmp(mode, "clamp"))	return GL_CLAMP_TO_EDGE;
		if (strcmp(mode, "mirror"))	return GL_MIRRORED_REPEAT;
		if (strcmp(mode, "repeat"))	return GL_REPEAT;
	}
	return GL_REPEAT;
}

GLint parse_format(const char *format)
{
	if (format)
	{
		if (strcmp(format, "red"))	return GL_RED;
		if (strcmp(format, "rgb"))	return GL_RGB;
		if (strcmp(format, "rgba"))	return GL_RGBA;
		if (strcmp(format, "srgb"))	return GL_SRGB;
		if (strcmp(format, "srgba"))	return GL_SRGB_ALPHA;
	}
	return GL_RGB;
}

Texture Texture::import(const char *URI)
{
	uri_t uri;
	if (!uri.parse(URI))
		return Texture::none;

	unsigned id;
	uvec2 size;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	// Set the texture wrapping and filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, parse_wrap_mode(uri.get("wrap_s")));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, parse_wrap_mode(uri.get("wrap_t")));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (uri.on_disk)
	{
		sf::Image image;
		if (!image.loadFromFile("Assets/" + uri.path))
		{
			Error::add(Error::USER, "Cannot import texture: " + uri.path);
			glCheck(glDeleteTextures(1, &id));
			return Texture::none;
		}

		image.flipVertically();
		size = uvec2(image.getSize().x, image.getSize().y);
		glTexImage2D(GL_TEXTURE_2D, 0, parse_format(uri.get("format")),
			size.x, size.y, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		glCheck(glDeleteTextures(1, &id));
		return Texture::none;
	}

	textures.reserve(id);
	textures[id].size = size;
	return Texture(id);
}

void Texture::clear()
{
	for (int i = 0; i < textures.size(); i++)
	{
		Texture t(i);
		if (t.is_valid()) t.destroy();
	}
	textures.clear();
}
