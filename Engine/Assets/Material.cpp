#include "Assets/Material.h"

std::weak_ptr<Material> Material::basic;

Material::Material(Program *_program):
	program(_program)
{
	// Create custom UBOs
	for (auto block : program->getBlocks())
	{
		if (block.binding <= 1)
			continue;
		ubos.emplace_back(UBO::create(block.binding, block.size));

		uint8_t *data = ubos.back().data;
		for (auto uniform : block.uniforms)
			uniforms.emplace(uniform.name, data + uniform.offset);
	}
}

Material::~Material()
{
	for (UBO& ubo : ubos)
		UBO::release(ubo);
}

MaterialRef Material::clone() const
{
	return std::shared_ptr<Material>(new Material(program));
}

void Material::bind() const
{
	GL::UseProgram(program->program);
	for (const UBO& ubo : ubos)
		ubo.bind();
}

MaterialRef Material::create(Program *program)
{
	return std::shared_ptr<Material>(new Material(program));
}

MaterialRef Material::getDefault()
{
	if (auto shared = basic.lock())
		return shared;
	auto shared = Material::create(Program::getDefault());
	basic = shared;

	return shared;
}

template<>
void Material::set(std::string name, Texture* value)
{
	GL::UseProgram(program->program);
	GLint loc = glGetUniformLocation(program->program, name.c_str());
	glCheck(glUniform1i(loc, 0));
	value->use();
}
