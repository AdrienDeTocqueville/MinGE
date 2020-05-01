#include <string.h>

#include "Graphics/Shaders/Material.inl"
#include "Graphics/Shaders/Shader.inl"
#include "Graphics/Shaders/Program.h"
#include "Graphics/Textures/Texture.h"

#include "Utility/Error.h"

multi_array_t<material_t> Material::materials;

Material Material::create(Shader *shader)
{
	uint32_t i = materials.add();
	materials.get<0>(i)->variant_hash = 0;
	materials.get<0>(i)->variant_idx = 0;
	materials.get<0>(i)->shader = shader;
	new (&materials.get<0>(i)->uniforms) std::vector<uint8_t>();
	materials.get<0>(i)->sync_uniforms();

	return Material(i);
}

Material Material::copy(Material src)
{
	uint32_t i = materials.add();
	std::memcpy(materials.get<0>(i), materials.get<0>(src.id()), sizeof(material_t));
	new (&materials.get<0>(i)->uniforms) std::vector<uint8_t>(materials.get<0>(src.id())->uniforms);

	return Material(i);
}

void Material::define(const std::vector<std::string> &macros)
{
	material_t *m = materials.get<0>(index);
	Shader *shader = m->shader;
	uint32_t hash = m->variant_hash;

	for (const std::string &macro : macros)
	{
		auto it = shader->macros.find(macro);
		if (it == shader->macros.end())
			return;

		hash = (hash & ~it->second.mask) | it->second.id;
	}
	m->update_variant(hash);
}

void Material::define(const std::string& macro)
{
	material_t *m = materials.get<0>(index);
	Shader *shader = m->shader;
	uint32_t hash = m->variant_hash;

	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return;

	hash = (hash & ~it->second.mask) | it->second.id;
	m->update_variant(hash);
}

void Material::undef(const std::string& macro)
{
	material_t *m = materials.get<0>(index);
	Shader *shader = m->shader;
	uint32_t hash = m->variant_hash;

	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return;

	hash = (hash & ~it->second.mask);
	m->update_variant(hash);
}

bool Material::defined(const std::string& macro) const
{
	material_t *m = materials.get<0>(index);
	Shader *shader = m->shader;

	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return false;

	return (m->variant_hash & ~it->second.mask) == it->second.id;
}


size_t Material::get_location(const std::string &name) const
{
	material_t *m = materials.get<0>(index);
	Shader *shader = m->shader;

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


const material_t *material_t::bound = nullptr;

void material_t::bind(RenderPass::Type pass) const
{
	auto *prgm = shader->update_builtins(variant_idx, pass);

	if (bound == this) return;
	else bound = this;

	int texture_slot = 0;
	for (const Program::Uniform &var : prgm->uniforms)
	{
		const void *data = uniforms.data() + var.offset;

		if (var.type == GL_SAMPLER_2D)
		{
			Texture *t = (Texture*)data;

			GL::ActiveTexture(GL_TEXTURE0 + texture_slot);
			glBindTexture(GL_TEXTURE_2D, t->id());
			set_uniform(var.location, texture_slot++);
		}
		else
			set_uniform(var.location, var.type, var.num, data);
	}
}
