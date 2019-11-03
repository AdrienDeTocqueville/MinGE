#include "Assets/Material.h"
#include "Utility/Error.h"

static const std::map<GLuint, uint32_t> uniform_type_size = {
	{GL_FLOAT,	sizeof(float)},
	{GL_FLOAT_VEC2, sizeof(vec2)},
	{GL_FLOAT_VEC3, sizeof(vec3)},
	{GL_FLOAT_VEC4, sizeof(vec4)},
	{GL_FLOAT_MAT3, sizeof(mat3)},
	{GL_FLOAT_MAT4, sizeof(mat4)},
	{GL_SAMPLER_2D, sizeof(Texture*)},
};

std::weak_ptr<Material> Material::basic;

Material::Material(Program *_program):
	program(_program)
{
	for (auto& u : program->getUniforms())
	{
		if (uniform_type_size.find(u.type) == uniform_type_size.end())
			Error::add(USER_ERROR, "Unsupprted uniform type");

		property_names[u.name] = properties.size();
		properties.push_back({u.type, u.location, uniforms.size()});
		uniforms.resize(uniforms.size() + uniform_type_size.at(u.type));
	}
}

MaterialRef Material::clone() const
{
	return std::shared_ptr<Material>(new Material(program));
}

void Material::bind() const
{
	GL::UseProgram(program->program);
	for (const Property& prop : properties)
	{
		const void *value = uniforms.data() + prop.offset;
		switch (prop.type)
		{
		case GL_SAMPLER_2D:
		{
			Texture *t = *(Texture**)value;
			t->use(0);
			glCheck(glUniform1i(prop.location, 0));
			break;
		}
		case GL_FLOAT:
			glCheck(glUniform1f(prop.location, *(float*)value));
			break;
		case GL_FLOAT_VEC2:
			glCheck(glUniform2fv(prop.location, 1, (float*)value));
			break;
		case GL_FLOAT_VEC3:
			glCheck(glUniform3fv(prop.location, 1, (float*)value));
			break;
		case GL_FLOAT_VEC4:
			glCheck(glUniform4fv(prop.location, 1, (float*)value));
			break;
		case GL_FLOAT_MAT3:
			glCheck(glUniformMatrix3fv(prop.location, 1, GL_FALSE, (float*)value));
			break;
		case GL_FLOAT_MAT4:
			glCheck(glUniformMatrix4fv(prop.location, 1, GL_FALSE, (float*)value));
			break;
		}
	}
}

MaterialRef Material::create(std::string name)
{
	return std::shared_ptr<Material>(new Material(Program::get(name)));
}

MaterialRef Material::getDefault()
{
	if (auto shared = basic.lock())
		return shared;
	auto shared = std::shared_ptr<Material>(new Material(Program::getDefault()));
	basic = shared;

	return shared;
}

template<>
void Material::set(uint32_t prop, Texture *value)
{
	*(Texture**)(uniforms.data() + properties[prop].offset) = value;
}
