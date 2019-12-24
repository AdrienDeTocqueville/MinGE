#include "Assets/Material.h"
#include "Assets/Texture.h"
#include "Assets/Shader.h"
#include "Assets/Shader.inl"

#include "Renderer/Program.h"

const Material *Material::bound = nullptr;
std::weak_ptr<Material> Material::standard;
std::vector<std::weak_ptr<Material>> Material::materials;

Material::Material(Shader *_shader):
	variant_hash(0), variant_idx(0),
	shader(_shader), id((uint32_t)materials.size())
{
	sync_uniforms();
}

Material::Material(const Material &material):
	variant_hash(material.variant_hash), variant_idx(material.variant_idx),
	shader(material.shader), uniforms(material.uniforms),
	id((uint32_t)materials.size())
{ }

Material::~Material()
{
	if (id != materials.size() - 1)
	{
		// Swap with last
		std::weak_ptr<Material> last = materials.back();
		auto shared = last.lock();

		shared->id = id;
		materials[id] = last;
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

void Material::define(std::string macro)
{
	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return;

	variant_hash = (variant_hash & ~it->second.mask) | it->second.id;
	variant_idx = shader->get_variant(variant_hash);
	sync_uniforms();
}

void Material::undef(std::string macro)
{
	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return;

	variant_hash = (variant_hash & ~it->second.mask);
	variant_idx = shader->get_variant(variant_hash);
	sync_uniforms();
}


size_t Material::getLocation(const std::string &name) const
{
	auto it = shader->uniforms_names.find(name);
	if (it != shader->uniforms_names.end())
		return it->second;

	std::cout << "Unknown uniform: " << name << std::endl;
	return -1;
}

MaterialRef Material::clone() const
{
	auto shared = MaterialRef(new Material(*this));
	materials.push_back(shared);
	
	return shared;
}

MaterialRef Material::create(std::string name)
{
	auto shared = MaterialRef(new Material(Shader::get(name)));
	materials.push_back(shared);
	
	return shared;
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

MaterialRef Material::get(uint32_t id)
{
	return materials[id].lock();
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

	if (Material::bound == this) Material::bound = NULL;
}

template<>
void Material::set(size_t location, Texture *value)
{
	if (value == NULL) value = Texture::getDefault();
	*(Texture**)(uniforms.data() + location) = value;

	if (Material::bound == this) Material::bound = NULL;
}
