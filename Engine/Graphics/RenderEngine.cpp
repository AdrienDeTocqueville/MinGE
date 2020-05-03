#include "Graphics/RenderEngine.h"

#include "Graphics/Debug.h"
#include "Graphics/Shaders/Shader.h"
#include "Graphics/Shaders/Material.inl"
#include "Graphics/Textures/Texture.h"


void RenderEngine::init()
{
	GL::init();
	Shader::setup_builtins();
	Debug::init();

	// TODO: destroy it
	Material mat = Material::create(Shader::standard());
	mat.set("color", vec3(0.8f));
	mat.set("metallic", 0.0f);
	mat.set("roughness", 0.5f);
}

void RenderEngine::destroy()
{
	Debug::destroy();

	Shader::clear();
	Texture::clear();
	// destroy meshes/materials ?
}
