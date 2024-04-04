#include "Render/GLDriver.h"
#include "Render/Texture/Texture.h"
#include "Utility/Log.h"
#include "IO/DataModel.h"

#define STB_IMAGE_IMPLEMENTATION
#include "Render/Texture/stb_image.h"

#include "Render/Texture/TextureImporter.inl"

array_list_t<Texture::texture_t> Texture::textures;

static void destroy_texture(UID32 object)
{
	Texture texture(object);
	ASSERT(texture.is_valid(), "Trying to destroy an invalid Texture handle");

	Texture::texture_t &data = Texture::textures[object.id()];
	glCheck(glDeleteTextures(1, &data.handle));

	data.destroy();
	Texture::textures.remove(object.id());
}

void Texture::destroy()
{
	destroy_texture(*this);
}

inline static int mipmap_count(vec2 size)
{
	return static_cast<int>(std::floor(std::log2(std::max(size.x, size.y)))) + 1;
}

static int create_internal(const Texture::settings_t &settings, Texture::texture_t &texture)
{
	glGenTextures(1, &texture.handle);
	glBindTexture(GL_TEXTURE_2D, texture.handle);

	// Set the texture wrapping and filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, settings.wrap_mode_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, settings.wrap_mode_t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, settings.filter_mode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, settings.filter_mode);

	texture.size = uvec2(settings.width, settings.height);
	int mips = settings.generate_mips ? mipmap_count(texture.size) : 1;
	glTexStorage2D(GL_TEXTURE_2D, mips, settings.format, texture.size.x, texture.size.y);

	return 0;
}

void Texture::set_pixel_data(color32_t *data)
{
	Texture::texture_t &tex = textures[id()];
	glTextureSubImage2D(tex.handle, 0, 0, 0, tex.size.x, tex.size.y,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateTextureMipmap(tex.handle);
}

Texture Texture::create(const settings_t &settings)
{
	uint32_t i = textures.add();
	Texture::texture_t &texture = textures[i];
	if (create_internal(settings, texture) != 0)
	{
		textures.remove(i);
		return Texture();
	}

	texture.create();
	if (settings.name != nullptr)
	{
		glCheck(glObjectLabel(GL_TEXTURE, texture.handle, (GLsizei)strlen(settings.name), settings.name));
		texture.name = strdup(settings.name);
	}

	return Texture::get(i);

}

static UID32 transfer(UID32 object, DataModel *model)
{
	if (!object.valid())
	{
		uint32_t i = Texture::textures.add();
		Texture::textures[i].create();
		object = Texture::get(i);
	}
	Texture texture = Texture(object);
	Texture::texture_t &data = Texture::textures[texture.id()];

	model->transfer("name", data.name);
	model->transfer("size", data.size);
	//model->transfer(data.pixels, data.size.x * data.size.y * sizeof(color32_t));

	return object;
}

const object_type_t Texture::type = []() {
	object_type_t t = INIT_ASSET_TYPE(Texture);

	t.destroy = destroy_texture;
	t.data = [](UID32 obj) { return (object_data_t*)&(Texture::textures[obj.id()]); };
	t.transfer = transfer;
	return t;
}();
