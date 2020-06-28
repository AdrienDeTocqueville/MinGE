#include "Render/Textures/Texture.h"
#include "Utility/Error.h"
#include "IO/json.hpp"
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

	free(textures.get<1>(id()));
	*textures.get<1>(id()) = NULL;
	++(*textures.get<2>(id()));
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

Texture Texture::load(const char *URI)
{
	uri_t uri;
	if (!uri.parse(URI))
		return Texture::none;

	unsigned handle;
	uvec2 size;
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);

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
			glCheck(glDeleteTextures(1, &handle));
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
		// TODO
		glCheck(glDeleteTextures(1, &handle));
		return Texture::none;
	}

	uint32_t i = textures.add();
	textures.get<0>()[i] = { handle, size };
	textures.get<1>()[i] = strdup(URI);

	return Texture(i, textures.get<2>()[i]);
}

Texture Texture::get(uint32_t i)
{
	if (textures.get<1>()[i] == NULL)
		return Texture::none;
	return Texture(i, textures.get<2>()[i]);
}

Texture Texture::find_by_handle(uint32_t handle)
{
	for (uint32_t i(1); i <= textures.size; i++)
	{
		if (textures.get<1>()[i] != NULL && textures.get<0>()[i].handle == handle)
			return Texture(i, textures.get<2>()[i]);
	}
	return Texture::none;
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

