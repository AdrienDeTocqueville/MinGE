#include "Assets/Texture.h"

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

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, image.getSize().x, image.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    textures[path] = this;
}

Texture::Texture(unsigned _width, unsigned _height):
    texture(0)
{
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture()
{
    glDeleteTextures(1, &texture);
    texture = 0;
}

/// Methods (public)
void Texture::use(unsigned _active) const
{
    glActiveTexture(GL_TEXTURE0 + _active);
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

void Texture::clear()
{
    for(auto& it: textures)
        delete it.second;

    textures.clear();
}


RenderBuffer::RenderBuffer(unsigned _width, unsigned _height, GLenum _format):
    width(_width), height(_height),
    renderBuffer(0), format(_format)
{
    glGenRenderbuffers(1, &renderBuffer);

    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, _format, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

RenderBuffer::~RenderBuffer()
{
    glDeleteRenderbuffers(1, &renderBuffer);
}

/// Methods (public)
void RenderBuffer::destroy()
{
    delete this;
}
