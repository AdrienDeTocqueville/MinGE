#include "Assets/Material.h"
#include "Assets/Program.h"

const Material *Material::bound = nullptr;
std::weak_ptr<Material> Material::basic;

Material::Material(Program *_program):
	program(_program)
{
	for (const auto& u : program->uniforms)
	{
		uniforms.resize(uniforms.size() + u.size);

		if (u.type == GL_SAMPLER_2D)
			set(uniforms.size() - u.size, (Texture*)NULL);
	}
}

Material::Material(const Material &material):
	program(material.program), uniforms(material.uniforms)
{ }

void Material::bind(RenderPass pass) const
{
	// TODO: handle multipass
	program->updateBuiltins();

	if (Material::bound == this)
		return;
	Material::bound = this;

	int texture_slot = 0;
	for (const Program::Uniform& uniform : program->uniforms)
	{
		const void *data = uniforms.data() + uniform.offset;

		if (uniform.type == GL_SAMPLER_2D)
		{
			Texture *t = *(Texture**)data;
			t->use(texture_slot);
			glCheck(glUniform1i(uniform.location, texture_slot++));
		}
		else
			set_uniform(uniform.location, uniform.type, data);
	}
}

bool Material::hasRenderPass(RenderPass pass) const
{
	// place holder
	return (pass == RenderPass::Forward);
}

size_t Material::getLocation(const std::string &name) const
{
	auto it = program->uniforms_names.find(name);
	if (it != program->uniforms_names.end())
		return program->uniforms[it->second].offset;

	std::cout << "Unknown uniform: " << name << std::endl;
	return -1;
}

MaterialRef Material::clone() const
{
	return std::shared_ptr<Material>(new Material(*this));
}

MaterialRef Material::create(std::string name)
{
	return MaterialRef(new Material(Program::get(name)));
}

MaterialRef Material::getDefault()
{
	if (auto shared = basic.lock())
		return shared;

	MaterialRef shared(new Material(Program::getDefault()));
	shared->set("ambient", vec3(0.3f));
	shared->set("diffuse", vec3(0.8f));
	shared->set("specular", vec3(0.0f));
	shared->set("exponent", 8.0f);

	basic = shared;
	return shared;
}

template<>
void Material::set(size_t location, Texture *value)
{
	if (value == NULL) value = Texture::getDefault();
	*(Texture**)(uniforms.data() + location) = value;
}
