#include "Assets/Texture.h"
#include "Utility/Error.h"

#include <SFML/Graphics/Image.hpp>

std::unordered_map<std::string, Texture*> Texture::textures;

Texture::Texture():
	texture(0)
{ }

Texture::Texture(std::string _path):
	texture(0)
{
	path = "Resources/" + _path;

	sf::Image image;
	if (!image.loadFromFile(path))
	{
		Error::add(MINGE_ERROR, "Texture::Texture() -> SFML failed to load file: " + path);
		return;
	}

	image.flipVertically();

	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	// Set the texture wrapping/filtering options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload data and build mip maps
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.getSize().x, image.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, image.getPixelsPtr());
	glGenerateMipmap(GL_TEXTURE_2D);

	textures[_path] = this;
}

Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}

void Texture::create(uvec2 _size)
{
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _size.x, _size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/// Methods (public)
void Texture::use(unsigned _active) const
{
	GL::ActiveTexture(GL_TEXTURE0 + _active);
	glBindTexture(GL_TEXTURE_2D, texture);
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


void RenderBuffer::create(uvec2 _size, GLenum _format)
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
