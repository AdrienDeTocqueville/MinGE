#include "Graphics/Textures/Texture.h"
#include "Utility/Error.h"
#include "IO/URI.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Graphics/Textures/stb_image.h"

struct texture_t
{
	uvec2 size;
	const char *URI;
	uint32_t gen;
};

const Texture Texture::none;
static std::vector<texture_t> textures = { texture_t { uvec2{0,0}, NULL, 1 } };


bool Texture::is_valid()
{
	return textures[id()].gen == gen();
}

uvec2 Texture::size()
{
	assert(id() && "Invalid texture handle");

	return textures[id()].size;
}

void Texture::destroy()
{
	assert(is_valid() && "Invalid texture handle");

	GLuint i = (GLuint)id();
	glCheck(glDeleteTextures(1, &i));
	textures[id()].gen++;
	textures[id()].URI = NULL;
}


GLint parse_wrap_mode(const char *mode)
{
	if (mode)
	{
		if (!strcmp(mode, "clamp"))	return GL_CLAMP_TO_EDGE;
		if (!strcmp(mode, "mirror"))	return GL_MIRRORED_REPEAT;
		if (!strcmp(mode, "repeat"))	return GL_REPEAT;
	}
	return GL_REPEAT;
}

GLint parse_format(const char *format)
{
	if (format)
	{
		if (!strcmp(format, "red"))	return GL_RED;
		if (!strcmp(format, "rgb"))	return GL_RGB;
		if (!strcmp(format, "rgba"))	return GL_RGBA;
		if (!strcmp(format, "srgb"))	return GL_SRGB;
		if (!strcmp(format, "srgba"))	return GL_SRGB_ALPHA;
	}
	return GL_RGB;
}

GLenum channels_to_format(int n)
{
	switch (n)
	{
	case 1:	 return GL_RED;
	case 2:	 return GL_RG;
	case 3:	 return GL_RGB;
	default: return GL_RGBA;
	}
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
		int x, y, n;
		unsigned char *data = stbi_load(uri.path.c_str(), &x, &y, &n, 0);
		if (data == NULL)
		{
			Error::add(Error::USER, "Cannot import texture: " + uri.path);
			glCheck(glDeleteTextures(1, &id));
			return Texture::none;
		}

		size = uvec2(x, y);
		glTexImage2D(GL_TEXTURE_2D, 0, parse_format(uri.get("format")), x, y, 0,
			channels_to_format(n), GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}
	else
	{
		glCheck(glDeleteTextures(1, &id));
		return Texture::none;
	}

	if (textures.size() <= id)
		textures.resize(id + 1);
	textures[id].size = size;
	textures[id].URI = URI;
	return Texture(id, textures[id].gen);
}

void Texture::clear()
{
	for (unsigned i = 0; i < textures.size(); i++)
	{
		GLuint id = i;
		if (glIsTexture(id))
			glCheck(glDeleteTextures(1, &id));
	}
	textures.clear();
}
