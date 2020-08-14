#include <string.h>

#include "Render/Shader/Material.inl"
#include "Render/Shader/Shader.inl"
#include "Render/Shader/Program.h"
#include "Render/Texture/Texture.h"

#include "Core/Asset.inl"
#include "Utility/Error.h"

#include "IO/URI.h"
#include "IO/json.hpp"

const Material Material::none;
multi_array_t<material_t, char*, uint8_t> Material::materials;


void Material::destroy()
{
	assert(is_valid() && "Invalid Material handle");

	auto *uniforms = &materials.get<0>(id())->uniforms;
	uniforms->~vector<uint8_t>();

	char **uri = materials.get<1>(id());
	free(*uri);
	*uri = NULL;

	++(*materials.get<2>(id()));
	materials.remove(id());
}


Material Material::load(const char *URI)
{
	uri_t uri;
	if (!uri.parse(URI))
		return Material::none;

	Shader *shader;
	const char *shader_path;
	if (!uri.try_get("shader", shader_path) || !(shader = Shader::load(shader_path)))
		return Material::none;

	uint32_t prev_size = materials.size;
	uint32_t i = materials.add();

	materials.get<0>(i)->variant_hash = 0;
	materials.get<0>(i)->variant_idx = 0;
	materials.get<0>(i)->shader = shader;
	new (&materials.get<0>(i)->uniforms) std::vector<uint8_t>();
	materials.get<0>(i)->sync_uniforms();

	materials.get<1>()[i] = strdup(URI);

	if (prev_size != materials.size)
		materials.get<2>()[i] = 0;

	return Material(i, materials.get<2>()[i]);
}

Material Material::copy(Material src)
{
	uint32_t prev_size = materials.size;
	uint32_t i = materials.add();

	std::memcpy(materials.get<0>(i), materials.get<0>(src.id()), sizeof(material_t));
	new (&materials.get<0>(i)->uniforms) std::vector<uint8_t>(materials.get<0>(src.id())->uniforms);

	materials.get<1>()[i] = strdup(src.uri());

	if (prev_size != materials.size)
		materials.get<2>()[i] = 0;

	return Material(i, materials.get<2>()[i]);
}

Material Material::get(uint32_t i)
{
	if (materials.get<1>()[i] == NULL)
		return Material::none;
	return Material(i, materials.get<2>()[i]);
}

void Material::reload(Shader *shader)
{
	Shader *reloaded = shader->reload();
	if (reloaded == NULL)
		return;

	for (uint32_t i(1); i <= materials.size; i++)
	{
		if (materials.get<0>(i)->shader == shader)
			materials.get<0>(i)->shader = reloaded;
	}
}

void Material::clear()
{
	for (uint32_t i(1); i <= materials.size; i++)
	{
		if (materials.get<1>()[i] == NULL)
			continue;

		auto *uniforms = &materials.get<0>(i)->uniforms;
		uniforms->~vector<uint8_t>();

		free(materials.get<1>()[i]);
	}
	materials.clear();
}


void Material::define(const std::vector<std::string> &macros)
{
	material_t *m = materials.get<0>(id());
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
	material_t *m = materials.get<0>(id());
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
	material_t *m = materials.get<0>(id());
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
	material_t *m = materials.get<0>(id());
	Shader *shader = m->shader;

	auto it = shader->macros.find(macro);
	if (it == shader->macros.end())
		return false;

	return (m->variant_hash & ~it->second.mask) == it->second.id;
}


size_t Material::get_location(const std::string &name) const
{
	material_t *m = materials.get<0>(id());
	Shader *shader = m->shader;

	auto it = shader->uniforms_names.find(name);

#ifdef DEBUG
	if (it == shader->uniforms_names.end())
	{
		Error::addf(Error::USER, "Unknown uniform: %s", name.c_str());
		return -1;
	}
#endif

	return it->second;
}


const void *material_t::bound = nullptr;

void material_t::bind(RenderPass::Type pass) const
{
	Program *prgm = shader->variants[variant_idx].passes[pass];
	GL::UseProgram(prgm->program);

	// works because sizeof(this) < RenderPass::Count
	if (bound == (uint8_t*)this + pass) return;
	else bound = (uint8_t*)this + pass;

	int texture_slot = 0;
	for (const Program::Uniform &var : prgm->uniforms)
	{
		const void *data = uniforms.data() + var.offset;

		if (var.type == GL_SAMPLER_2D)
		{
			GL::ActiveTexture(GL_TEXTURE0 + texture_slot);
			set_uniform(var.location, texture_slot++);

			Texture *t = (Texture*)data;
			glBindTexture(GL_TEXTURE_2D, t->handle());
		}
		else
			set_uniform(var.location, var.type, var.num, data);
	}
}


/// Serialization
using namespace nlohmann;
void material_save(json &dump)
{
	Asset::save(dump, Material::materials, Material::get);
}

void material_load(const json &dump)
{
	Asset::load<Material, 1, 2>(dump, Material::materials, Material::materials.get<0>());
}

const asset_type_t Material::type = []() {
	asset_type_t t{ NULL };
	t.name = "Material";

	t.save = material_save;
	t.load = material_load;
	return t;
}();
