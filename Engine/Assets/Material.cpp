#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

#include "Renderer/Program.h"
#include "Utility/Error.h"

const Material *Material::bound = nullptr;
std::weak_ptr<Material> Material::standard;
std::vector<Material*> Material::materials;

Material::Material(Shader *_shader):
	variant_hash(0), variant_idx(0), shader(_shader)
{
	compute_id(0);
	sync_uniforms();
}

Material::Material(const Material &material):
	variant_hash(material.variant_hash), variant_idx(material.variant_idx),
	shader(material.shader), uniforms(material.uniforms)
{
	compute_id(material.id);
}

Material::~Material()
{
	for (size_t j = id + 1; j < materials.size(); ++j)
	{
		materials[j]->id = j - 1;
		materials[j - 1] = materials[j];
	}
	materials.pop_back();
}

void Material::bind(RenderPass::Type pass) const
{
	auto *prgm = shader->updateBuiltins(variant_idx, pass);

	if (Material::bound == this)
		return;
	Material::bound = this;

	int texture_slot = 0;
	for (const Program::Uniform &var : prgm->uniforms)
	{
		const void *data = uniforms.data() + var.offset;

		if (var.type == GL_SAMPLER_2D)
		{
			Texture *t = *(Texture**)data;
			t->use(texture_slot);
			set_uniform(var.location, texture_slot++);
		}
		else
			set_uniform(var.location, var.type, var.num, data);
	}
}

bool Material::has_pass(RenderPass::Type pass) const
{
	return shader->passes[pass].exists;
}

void Material::reload()
{
	// TODO
}

void Material::define(const std::vector<std::string> &macros)
{
	for (const std::string &macro : macros)
	{
		auto it = shader->macros.find(macro);
		if (it == shader->macros.end())
			return;

		variant_hash = (variant_hash & ~it->second.mask) | it->second.id;
	}
	update_variant();
}

void Material::define(std::string macro)
{
	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return;

	variant_hash = (variant_hash & ~it->second.mask) | it->second.id;
	update_variant();
}

void Material::undef(std::string macro)
{
	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return;

	variant_hash = (variant_hash & ~it->second.mask);
	update_variant();
}

bool Material::ifdef(std::string macro) const
{
	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return false;

	return (variant_hash & ~it->second.mask) == it->second.id;
}


size_t Material::get_location(const std::string &name) const
{
	auto it = shader->uniforms_names.find(name);

#ifdef DEBUG
	if (it == shader->uniforms_names.end())
	{
		Error::add(Error::USER, "Unknown uniform: " + name);
		return -1;
	}
#endif

	return it->second;
}

void Material::update_variant()
{
	variant_idx = shader->get_variant(variant_hash);
	sync_uniforms();

	/// Update id

	// If not first
	if (id)
	{
		// Search on the left
		size_t i = id - 1;
		while (i != 0 &&
			materials[i]->variant_idx != variant_idx &&
			materials[i]->shader == shader)
			--i;

		for (size_t j = id; j > i; --j)
		{
			materials[j - 1]->id = j;
			materials[j] = materials[j - 1];
		}
		this->id = i;
		materials[i] = this;
		return;
	}
	// If not last
	if (id + 1 != materials.size())
	{
		// Search on the right
		size_t i = id + 1;
		while (i < materials.size() &&
			materials[i]->variant_idx != variant_idx &&
			materials[i]->shader == shader)
			++i;

		for (size_t j = id + 1; j < i; ++j)
		{
			materials[j]->id = j - 1;
			materials[j - 1] = materials[j];
		}
		this->id = i - 1;
		materials[i - 1] = this;
		return;
	}
}

void Material::compute_id(size_t i)
{
	// Find first material sharing same shader
	while (i != materials.size() &&
		materials[i]->shader != shader)
		++i;

	// Found ?
	if (i != materials.size())
	{
		// Find first using same variant
		while (i != materials.size() &&
			materials[i]->variant_idx != variant_idx &&
			materials[i]->shader == shader)
			++i;

		materials.push_back(NULL);
		for (size_t j = materials.size() - 1; j > i; --j)
		{
			materials[j - 1]->id = j;
			materials[j] = materials[j - 1];
		}
		this->id = i;
		materials[i] = this;
	}
	else
	{
		id = materials.size();
		materials.push_back(this);
	}
}

MaterialRef Material::create(std::string name)
{
	return MaterialRef(new Material(Shader::get(name)));
}

MaterialRef Material::getDefault()
{
	if (auto shared = standard.lock())
		return shared;

	MaterialRef shared = Material::create("standard");
	shared->set("color", vec3(0.8f));
	shared->set("metallic", 0.0f);
	shared->set("roughness", 0.5f);

	standard = shared;
	return shared;
}

MaterialRef Material::clone() const
{
	return MaterialRef(new Material(*this));
}

void Material::sync_uniforms()
{
	size_t old_size = uniforms.size();
	uniforms.resize(shader->uniform_offset);

	// Initialize textures with default value
	for (int i(0); i < RenderPass::Count; i++)
	{
		if (Program *prgm = shader->variants[variant_idx].passes[i])
		{
			for (const Program::Uniform &var : prgm->uniforms)
			{
				if (var.offset >= old_size && var.type == GL_SAMPLER_2D)
					set(var.offset, (Texture*)NULL);
			}
		}
	}
}

template<>
void Material::set(size_t location, const Texture *value)
{
	if (value == NULL) value = Texture::getDefault();
	*(Texture**)(uniforms.data() + location) = (Texture*)value;
}
