#include "Render/GLDriver.h"
#include "Render/Textures/Texture.h"
#include "Utility/Error.h"

#include "IO/json.hpp"
#include "IO/Input.h"
#include "IO/URI.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Render/Textures/stb_image.h"


const Texture Texture::none;
// {id, size}, uri, gen
multi_array_t<Texture::texture_t, char*, uint8_t> Texture::textures;


void Texture::destroy()
{
	assert(is_valid() && "Invalid Texture handle");

	GLuint *handle = &textures.get<0>()[id()].handle;
	glCheck(glDeleteTextures(1, handle));

	char **uri = textures.get<1>(id());
	uint8_t *gen = textures.get<2>(id());

	free(*uri);
	*uri = NULL;
	++(*gen);
}


static GLint parse_wrap_mode(const char *wrap)
{
	if (wrap == NULL) return GL_REPEAT;

	else if (!strcmp(wrap, "clamp"))	return GL_CLAMP_TO_EDGE;
	else if (!strcmp(wrap, "mirror"))	return GL_MIRRORED_REPEAT;
	else if (!strcmp(wrap, "repeat"))	return GL_REPEAT;

	else Error::addf(Error::USER, "Wrap mode is not supported: %s", wrap);
}

static GLint parse_filter(const char *filter)
{
	if (filter == NULL) return GL_LINEAR;

	else if (!strcmp(filter, "nearest"))	return GL_NEAREST;
	else if (!strcmp(filter, "linear"))	return GL_LINEAR;

	else Error::addf(Error::USER, "Filter mode is not supported: %s", filter);
}

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

	else Error::addf(Error::USER, "Format is not supported: %s", format);
}

inline static GLenum channels_to_format(int n)
{
	switch (n)
	{
	case 1:  return GL_RED;
	case 2:  return GL_RG;
	case 3:  return GL_RGB;
	default: return GL_RGBA;
	}
}

inline static int mipmap_count(vec2 size)
{
	return static_cast<int>(std::floor(std::log2(std::max(size.x, size.y)))) + 1;
}

static int load_internal(const char *URI, unsigned &handle, uvec2 &size)
{
	uri_t uri;
	if (!uri.parse(URI))
		return -1;

	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

	// Set the texture wrapping and filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, parse_wrap_mode(uri.get("wrap_s")));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, parse_wrap_mode(uri.get("wrap_t")));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, parse_filter(uri.get("min_filter")));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, parse_filter(uri.get("max_filter")));

	bool use_mips = uri.get_or_default("mips", false);
	GLint internal_format = parse_internal_format(uri.get("format"));

	if (uri.on_disk)
	{
		int x, y, n;
		unsigned char *data = stbi_load(uri.path.c_str(), &x, &y, &n, 0);
		if (data == NULL) { glCheck(glDeleteTextures(1, &handle)); return -2; }

		size = uvec2(x, y);
		int mipmaps = use_mips ? mipmap_count(size) : 1;
		glTexStorage2D(GL_TEXTURE_2D, mipmaps, internal_format, x, y);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, x, y, channels_to_format(n), GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}
	else
	{
		vec2 scale;
		if (uri.try_get("scale", scale))
		{
			vec2 ws = Input::window_size();
			size = uvec2(ws.x * scale.x, ws.y * scale.y);
		}
		else
			size = uvec2(uri.get_or_default("width", 1), uri.get_or_default("height", 1));

		int mipmaps = use_mips ? mipmap_count(size) : 1;
		glTexStorage2D(GL_TEXTURE_2D, mipmaps, internal_format, size.x, size.y);
	}

	if (use_mips)
		glGenerateMipmap(GL_TEXTURE_2D);

	const char *label;
	int label_len;
	uri.extract_label(URI, label, label_len);
	glCheck(glObjectLabel(GL_TEXTURE, handle, label_len, label));

	return 0;
}

Texture Texture::load(const char *URI)
{
	uvec2 size;
	unsigned handle;
	if (load_internal(URI, handle, size))
	{
		Error::addf(Error::USER, "Cannot import texture: %s", URI);
		return Texture::none;
	}

	uint32_t i = textures.add();
	textures.get<0>()[i] = { handle, size };
	textures.get<1>()[i] = strdup(URI);

	return Texture(i, textures.get<2>()[i]);
}

void Texture::reload(const char *URI)
{
	assert(is_valid() && "Invalid Texture handle");

	uvec2 size;
	unsigned handle;
	if (load_internal(URI, handle, size))
		return;

	glCheck(glDeleteTextures(1, &textures.get<0>(id())->handle));
	free(textures.get<1>()[id()]);

	textures.get<0>()[id()] = { handle, size };
	textures.get<1>()[id()] = strdup(URI);
}

Texture Texture::get(uint32_t i)
{
	if (textures.get<1>()[i] == NULL)
		return Texture::none;
	return Texture(i, textures.get<2>()[i]);
}

void Texture::clear()
{
	for (uint32_t i(1); i <= textures.size; i++)
	{
		if (textures.get<1>()[i] == NULL)
			continue;

		GLuint *handle = &textures.get<0>()[i].handle;
		glCheck(glDeleteTextures(1, handle));

		free(textures.get<1>()[i]);
	}
	textures.clear();
}


/// Serialization
using namespace nlohmann;
void texture_save(json &dump)
{
	uint32_t max_id = 0;
	json textures = json::array();
	textures.get_ptr<json::array_t*>()->reserve(Texture::textures.size);
	for (uint32_t i(1); i <= Texture::textures.size; i++)
	{
		auto texture = Texture::get(i);
		if (texture == Texture::none)
			continue;

		max_id = i;
		json texture_dump = json::object();
		texture_dump["uint"] = texture.uint();
		texture_dump["uri"] = texture.uri();
		textures.push_back(texture_dump);
	}

	dump["max_id"] = max_id;
	dump["textures"].swap(textures);
}

void texture_load(const json &dump)
{
	uint32_t final_slot = 1;
	uint32_t max_id = dump["max_id"].get<uint32_t>();

	// Clear free list
	Texture::textures.init(max_id);
	for (uint32_t i(1); i <= max_id; i++)
		Texture::textures.get<1>()[i] = NULL;

	// Populate
	const json &textures = dump["textures"];
	for (auto it = textures.rbegin(); it != textures.rend(); ++it)
	{
		UID32 uid = it.value()["uint"].get<uint32_t>();

		auto *data = Texture::textures.get<0>();
		if (uid.id() == 1) final_slot = *(uint32_t*)(data + uid.id());
		else *(uint32_t*)(data + uid.id() - 1) = *(uint32_t*)(data + uid.id());

		Texture::textures.next_slot = uid.id();
		Texture::load(it.value()["uri"].get<std::string>().c_str());
		Texture::textures.get<2>()[uid.id()] = uid.gen();
	}
	Texture::textures.next_slot = final_slot;
}

const asset_type_t Texture::type = []() {
	asset_type_t t{ NULL };
	t.name = "Texture";

	t.save = texture_save;
	t.load = texture_load;
	return t;
}();

