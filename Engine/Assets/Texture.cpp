#include "Assets/Texture.h"
#include "Utility/Error.h"

#include <SFML/Graphics/Image.hpp>

std::unordered_map<std::string, Texture*> Texture::textures;

Texture::Texture(Texture &&t):
	id(t.id)
{
	t.id = 0;
}

Texture::Texture(std::string _path):
	id(0)
{
	path = "Assets/" + _path;

	sf::Image image;
	if (!image.loadFromFile(path))
	{
		Error::add(Error::MINGE, "Texture::Texture() -> SFML failed to load file: " + path);
		return;
	}

	image.flipVertically();

	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	// Set the texture wrapping/filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload data and build mip maps
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	glGenerateMipmap(GL_TEXTURE_2D);

	textures[_path] = this;
}

Texture::~Texture()
{
	glCheck(glDeleteTextures(1, &id));
}

void Texture::create(uvec2 _size, int _format, int _type)
{
	glCheck(glGenTextures(1, &id));

	glCheck(glBindTexture(GL_TEXTURE_2D, id));

	glCheck(glTexImage2D(GL_TEXTURE_2D, 0, _format, _size.x, _size.y, 0, _format, _type, nullptr));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

/// Methods (public)
void Texture::use(unsigned _active) const
{
	GL::ActiveTexture(GL_TEXTURE0 + _active);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::destroy()
{
	if (!path.empty())
		textures.erase(textures.find(path));

	delete this;
}

/// Methods (static)
Texture* Texture::get(std::string _path)
{
	auto it = textures.find(_path);
	if (it == textures.end())
		return new Texture(_path);
	else
		return it->second;
}

Texture* Texture::getDefault()
{
	return Texture::get("Textures/white.png");
}

void Texture::clear()
{
	for(auto& it: textures)
		delete it.second;

	textures.clear();
}


void RenderBuffer::create(uvec2 _size, int _format)
{
	size = _size;
	glGenRenderbuffers(1, &renderBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, _format, size.x, size.y);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer()
{
	if (renderBuffer)
		glDeleteRenderbuffers(1, &renderBuffer);
}
