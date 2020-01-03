#ifndef TEXTURE_H
#define TEXTURE_H

#include "Utility/helpers.h"
#include "Renderer/GLDriver.h"

class Texture
{
public:
	Texture(): id(0) {}
	Texture(Texture &&t);
	~Texture();

	void create(uvec2 _size, int _format = GL_RGB, int _type = GL_UNSIGNED_BYTE);
	void use(unsigned _active = 0) const;
	void destroy();

	/// Methods (static)
	static Texture* get(std::string _path);
	static Texture* getDefault();
	static void clear();

	/// Getter
	unsigned getId() const { return id; }

private:
	/// Constructors
	Texture(std::string _path);

	/// Operator
	Texture operator=(Texture _original) = delete;

	/// Attributes (static)
	static std::unordered_map<std::string, Texture*> textures;

	/// Attributes (private)
	std::string path;
	unsigned id;
};

class RenderBuffer
{
public:
	void create(uvec2 _size, int _format);
	unsigned getId() const { return renderBuffer; }

	~RenderBuffer();
private:
	RenderBuffer operator=(RenderBuffer) = delete;

	/// Attributes (private)
	uvec2 size;
	unsigned renderBuffer = 0;
};

#endif // TEXTURE_H
