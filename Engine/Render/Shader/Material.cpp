#include <string.h>

#include "Render/Shader/Material.inl"
#include "Render/Shader/Shader.inl"
#include "Render/Shader/Program.h"
#include "Render/Texture/Texture.h"

#include "Core/Asset.inl"
#include "Core/Serialization.h"
#include "Utility/Error.h"
#include "IO/URI.h"

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


Material Material::load(const char *URI, uint32_t variant)
{
	uri_t uri;
	if (!uri.parse(URI))
		return Material::none;

	Shader *shader;
	const char *shader_path;
	if (!uri.try_get("shader", shader_path) || !(shader = Shader::load(shader_path)))
		return Material::none;

	uint32_t i = materials.add();
	material_t *material = materials.get<0>() + i;

	material->shader = shader;
	new (&material->uniforms) std::vector<uint8_t>();
	material->update_variant(variant);

	materials.get<1>()[i] = strdup(URI);

	return Material(i, materials.get<2>()[i]);
}

Material Material::copy(Material src)
{
	uint32_t i = materials.add();

	std::memcpy(materials.get<0>(i), materials.get<0>(src.id()), sizeof(material_t));
	new (&materials.get<0>(i)->uniforms) std::vector<uint8_t>(materials.get<0>(src.id())->uniforms);

	materials.get<1>()[i] = strdup(src.uri());

	return Material(i, materials.get<2>()[i]);
}

Material Material::get(uint32_t i)
{
	return ASSET_GET(Material, 1, 2, materials, i);
}

void Material::reload(Shader *shader)
{
	Shader *reloaded = shader->reload();
	if (reloaded == NULL)
		return;

	for (uint32_t i(1); i <= materials.size; i++)
	{
		if (materials.get<0>(i)->shader == shader)
		{
			materials.get<0>(i)->shader = reloaded;
			materials.get<0>(i)->update_variant(0);
		}
	}
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


/// Asset type
using namespace nlohmann;
void material_save(json &dump)
{
	Asset::save(dump, Material::materials, Material::get, [](uint32_t i, json &dump) {
		material_t *material = Material::materials.get<0>(i);
		dump["variant"] = material->variant_hash;
		// Save uniforms
		json u = json::object();
		Shader *shader = material->shader;
		for (const auto &it : shader->uniforms_names)
		{
			Program::Uniform *uniform = NULL;
			for (int v = 0; v < shader->variants.size(); v++)
			for (int p = 0; p < RenderPass::Count; p++)
			{
				auto *passes = shader->variants[v].passes;
				if (passes[p] != NULL)
				for (auto &uni : passes[p]->uniforms)
				{
					if (uni.offset == it.second)
					{
						uniform = &uni;
						goto serialize_uniform;
					}
				}
			}
			continue;

			serialize_uniform:
			if (uniform->offset >= material->uniforms.size()) continue;
			const void *data = material->uniforms.data() + uniform->offset;

			json v;
			switch (uniform->type)
			{
			case GL_INT:	    v = *(int*)data; break;
			case GL_FLOAT:	    v = *(float*)data; break;
			case GL_FLOAT_VEC2: v = ::to_json(*(vec2*)data); break;
			case GL_FLOAT_VEC3: v = ::to_json(*(vec3*)data); break;
			case GL_FLOAT_VEC4: v = ::to_json(*(vec4*)data); break;
			case GL_FLOAT_MAT3: v = ::to_json(*(mat3*)data); break;
			case GL_FLOAT_MAT4: v = ::to_json(*(mat4*)data); break;
			case GL_SAMPLER_2D: v = ((Texture*)data)->uint(); break;
			default: continue;
			}

			json final = json::object();
			final["type"] = uniform->type;
			final["value"].swap(v);
			u[it.first].swap(final);
		}
		dump["uniforms"].swap(u);
	});
}

void material_load(const json &dump)
{
	Asset::load<Material, 1, 2>(dump, Material::materials, [](const json &dump) {
		auto m = Material::load(dump["uri"].get<std::string>().c_str(), dump["variant"]);

		// Load uniforms
		material_t *material = Material::materials.get<0>(m.id());
		Shader *shader = material->shader;
		const json &data = dump["uniforms"];
		for (auto it = data.begin(); it != data.end(); ++it)
		{
			auto u = shader->uniforms_names.find(it.key());
			int type = it.value()["type"];
			if (u != shader->uniforms_names.end())
			{
				void *data = material->uniforms.data() + u->second;

				#define LOAD(type) *(type*)data = ::to_##type(it.value()["value"])
				switch (type)
				{
				case GL_INT:	    LOAD(int); break;
				case GL_FLOAT:	    LOAD(float); break;
				case GL_FLOAT_VEC2: LOAD(vec2); break;
				case GL_FLOAT_VEC3: LOAD(vec3); break;
				case GL_FLOAT_VEC4: LOAD(vec4); break;
				case GL_FLOAT_MAT3: LOAD(mat3); break;
				case GL_FLOAT_MAT4: LOAD(mat4); break;
				case GL_SAMPLER_2D:
				    *(Texture*)data = Texture::get(it.value()["value"]);
				    break;
				default: continue;
				}
			}
		}
	});
}

void material_clear()
{
	Asset::clear<1>(Material::materials, [](int i) {
		auto *uniforms = &Material::materials.get<0>(i)->uniforms;
		uniforms->~vector<uint8_t>();
	});
}

const asset_type_t Material::type = []() {
	asset_type_t t{ NULL };
	t.name = "Material";

	t.save = material_save;
	t.load = material_load;
	t.clear = material_clear;
	return t;
}();
