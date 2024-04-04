#include <cstring>

#include "Render/Shader/Material.inl"
#include "Render/Shader/Shader.inl"
#include "Render/Shader/Program.h"
#include "Render/Texture/Texture.h"

#include "Core/Serialization.h"
#include "Utility/Log.h"
#include "IO/URI.h"

array_list_t<Material::material_t> Material::materials;

static void destroy_material(UID32 object)
{
	Material material(object);
	ASSERT(material.is_valid(), "Trying to destroy an invalid Material handle");

	Material::material_t &data = Material::materials[object.id()];
	data.uniforms.~vector<uint8_t>();

	data.destroy();
	Material::materials.remove(object.id());
}

void Material::destroy()
{
	destroy_material(*this);
}

Material Material::create(Material src)
{
	uint32_t i = materials.add();
	material_t &material = materials[i];

	std::memcpy(&material, &(materials[src.id()]), sizeof(material_t));
	new (&material.uniforms) std::vector<uint8_t>(materials[src.id()].uniforms);

	material.name = nullptr;
	return Material::get(i);
}

Material Material::create(Shader shader, uint32_t variant)
{
	uint32_t i = materials.add();
	material_t &material = materials[i];

	material.shader = shader;
	new (&material.uniforms) std::vector<uint8_t>();
	material.update_variant(variant);

	material.name = nullptr;
	return Material::get(i);
}

void Material::define(const std::vector<std::string> &macros)
{
	material_t &m = materials[id()];
	Shader shader = m.shader;
	uint32_t hash = m.variant_hash;

	const auto &shader_macros = shader.get_data().macros;
	for (const std::string &macro : macros)
	{
		auto it = shader_macros.find(macro);
		if (it == shader_macros.end())
			continue;

		hash = (hash & ~it->second.mask) | it->second.id;
	}
	m.update_variant(hash);
}

void Material::define(const std::string& macro)
{
	material_t &m = materials[id()];
	Shader shader = m.shader;
	uint32_t hash = m.variant_hash;

	auto it = shader.get_data().macros.find(macro);
	if (it == shader.get_data().macros.end())
		return;

	hash = (hash & ~it->second.mask) | it->second.id;
	m.update_variant(hash);
}

void Material::undef(const std::string& macro)
{
	material_t &m = materials[id()];
	Shader shader = m.shader;
	uint32_t hash = m.variant_hash;

	auto it = shader.get_data().macros.find(macro);
	if (it == shader.get_data().macros.end())
		return;

	hash = (hash & ~it->second.mask);
	m.update_variant(hash);
}

bool Material::defined(const std::string& macro) const
{
	material_t &m = materials[id()];
	Shader shader = m.shader;

	auto it = shader.get_data().macros.find(macro);
	if (it == shader.get_data().macros.end())
		return false;

	return (m.variant_hash & ~it->second.mask) == it->second.id;
}


size_t Material::get_location(const std::string &name) const
{
	material_t &m = materials[id()];
	Shader shader = m.shader;

	auto it = shader.get_data().uniforms_names.find(name);

#ifdef DEBUG
	if (it == shader.get_data().uniforms_names.end())
	{
		Log::addf(Log::Error, "Unknown uniform: %s", name.c_str());
		return -1;
	}
#endif

	return it->second;
}


const void *Material::material_t::bound = nullptr;

void Material::material_t::bind(RenderPass::Type pass) const
{
	Program *prgm = shader.get_data().variants[variant_idx].passes[pass];
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


const object_type_t Material::type = []() {
	object_type_t t = INIT_ASSET_TYPE(Material);

	t.destroy = destroy_material;
	return t;
}();

