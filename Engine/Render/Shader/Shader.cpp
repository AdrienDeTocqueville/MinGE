#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <glm/gtc/integer.hpp>

#include "Render/Shader/Shader.inl"
#include "Render/Shader/Program.h"
#include "Render/Texture/Texture.h"
#include "Render/GLDriver.h"

#include "Utility/stb_sprintf.h"
#include "Utility/Error.h"
#include "Memory/Memory.h"
#include "IO/json.hpp"
#include "IO/URI.h"

using json = nlohmann::json;


static const std::unordered_map<std::string, RenderPass::Type> pass_type = {
	{"shadow",	RenderPass::Shadow},
	{"depth",	RenderPass::Depth},
	{"forward",	RenderPass::Forward},
};

static const std::unordered_map<GLuint, uint8_t> uniform_type_size = {
	{GL_INT,	(uint8_t)sizeof(int)},
	{GL_FLOAT,	(uint8_t)sizeof(float)},
	{GL_FLOAT_VEC2, (uint8_t)sizeof(vec2)},
	{GL_FLOAT_VEC3, (uint8_t)sizeof(vec3)},
	{GL_FLOAT_VEC4, (uint8_t)sizeof(vec4)},
	{GL_FLOAT_MAT3, (uint8_t)sizeof(mat3)},
	{GL_FLOAT_MAT4, (uint8_t)sizeof(mat4)},
	{GL_SAMPLER_2D, (uint8_t)sizeof(Texture)},
};

static const std::unordered_map<GLuint, const char*> uniform_type_names = {
	{GL_INT,	"int"},
	{GL_FLOAT,	"float"},
	{GL_FLOAT_VEC2, "vec2"},
	{GL_FLOAT_VEC3, "vec3"},
	{GL_FLOAT_VEC4, "vec4"},
	{GL_FLOAT_MAT3, "mat3"},
	{GL_FLOAT_MAT4, "mat4"},
	{GL_SAMPLER_2D, "sampler2D"},
};


// JSON helper
static inline std::string get_or_default(const json &n, const char *prop)
{
	if (!n.contains(prop)) return "";
	return n[prop];
}



static std::unordered_map<const char*, Shader*> shaders;

Shader* Shader::load(const char *URI)
{
	auto it = shaders.find(URI);
	if (it != shaders.end())
		return it->second;

	uri_t uri;
	if (!uri.parse(URI))
		return nullptr;

	const char *path = uri.path.c_str();
	if (!uri.on_disk)
	{
		static char buf[256];
		const char *name = strchr(path, '/');
		if (name++ == NULL) return nullptr;

		stbsp_snprintf(buf, sizeof(buf), "Assets/Shaders/%s.json", name);
		path = buf;
	}

	Shader *s = new Shader();
	uri.extract_label(URI, s->label, s->label_len);
	if (!s->load_json(path))
	{
		delete s;
		return nullptr;
	}
	shaders.emplace(URI, s);
	s->URI = URI;

	return s;
}

void Shader::clear()
{
	for (auto &entry: shaders)
		delete entry.second;
}

Shader::~Shader()
{
	std::unordered_set<Program*> freed;

	for (Shader::Variant &variant : variants)
	{
		for (int i(0); i < RenderPass::Count; i++)
		{
			if (freed.find(variant.passes[i]) != freed.end())
				continue;
			freed.emplace(variant.passes[i]);
			delete variant.passes[i];
		}
	}
}

Shader *Shader::reload()
{
	shaders.erase(URI);

	if (Shader *reloaded = Shader::load(URI))
	{
		/*
		std::unordered_map<std::string, size_t> uniforms_names_save;

		free_variants(variants);
		variants.clear();
		variant_idx.clear();
		macros.clear();
		std::swap(uniforms_names, uniforms_names_save);
		*/
		delete this;
		return reloaded;
	}
	else
	{
		shaders.emplace(URI, this);
		return NULL;
	}
}

/// Variant stuff
bool Shader::load_json(const char *path)
{
	// Clear state
	for (int i(0); i < RenderPass::Count; i++)
		passes[i].exists = false;
	next_bit = 0;
	uniform_offset = 0;

	// Parse file
	std::ifstream file(path);
	if (!file)
	{
		Error::addf(Error::FILE_NOT_FOUND, "Shader file not found: %s", path);
		return false;
	}

	json root;
	file >> root;

	// Parse passes
	if (!root.contains("passes"))
		return false;

	auto PASSES = root["passes"];
	for (json::iterator it = PASSES.begin(); it != PASSES.end(); ++it)
	{
		std::string pass_name = it.key();
		json PASS = it.value();

		auto type = pass_type.find(pass_name);
		if (type == pass_type.end())
		{
			Error::add(Error::USER, "Unknow pass type '" + pass_name + "'. Skipping...");
			continue;
		}

		passes[type->second].exists = true;
		passes[type->second].mask = 0;

		// Parse variants
		for (auto &macro : PASS["macros"])
		{
			if (macro.is_array())
			{
				uint32_t count = (uint32_t)macro.size();
				uint32_t count_align = mem::next_power_of_two(count);
				uint32_t mask = (count_align - 1) << next_bit;
				passes[type->second].mask |= mask;


				// Assume that if first macro is new, all are
				if (count && macros.find(macro[0].get<std::string>()) == macros.end())
				{
					for (uint32_t i = 0; i < count; i++)
					{
						std::string macro_name = macro[i].get<std::string>();
						macros.emplace(macro_name, Macro{mask, i << next_bit});
					}
					next_bit += glm::log2(count_align);
				}
			}
			else if (macro.is_string())
			{
				uint32_t mask = (uint32_t)(1 << next_bit);
				passes[type->second].mask |= mask;

				std::string macro_name = macro.get<std::string>();
				if (macros.find(macro_name) == macros.end())
				{
					macros.emplace(macro_name, Macro{mask, mask});
					next_bit++;
				}
				// No else: assume redefinition is correct
			}
			else
			{
				Error::add(Error::USER, "Invalid macro");
				return false;
			}
		}

		// Load stages
		if (PASS.contains("stages"))
		{
			json STAGES = PASS["stages"];
			auto &sources = passes[type->second].sources;

			sources.vertex = get_or_default(STAGES, "vertex");
			sources.tess_ctrl = get_or_default(STAGES, "tess-ctrl");
			sources.tess_eval = get_or_default(STAGES, "tess-eval");
			sources.geometry = get_or_default(STAGES, "geometry");
			sources.fragment = get_or_default(STAGES, "fragment");
		}
	}

	get_variant(0);

	return true;
}

uint32_t Shader::get_variant(uint32_t hash)
{
	auto it = variant_idx.find(hash);
	if (it != variant_idx.end())
		return it->second;

	variants.push_back({ 0 });
	Variant &variant = variants.back();

	// Generate #preprocessor directives
	std::string defines = "#version 430 core\n";
	for (auto &macro : macros)
	{
		if ((macro.second.mask & hash) == macro.second.id)
		{
			defines += "#define ";
			defines += macro.first;
			defines += "\n";
		}
	}

	// Compile shaders
	for (int i(0); i < RenderPass::Count; i++)
	{
		if (!passes[i].exists)
			continue;

		// Find if we already compiled this variant
		bool found = false;
		uint32_t mask = passes[i].mask;
		uint32_t pass_hash = hash & mask;
		for (auto hash_id : variant_idx)
		{
			if (pass_hash == (hash_id.first & mask))
			{
				variant.passes[i] = variants[hash_id.second].passes[i];
				found = true;
				break;
			}
		}

		// If not, compile it
		if (!found)
		{
			variant.passes[i] = new Program(passes[i].sources, (RenderPass::Type)i, defines.c_str());
			variant.passes[i]->label(label, label_len);
			load_uniforms(variant.passes[i]);
		}
	}

	variant_idx.emplace(hash, (uint32_t)variants.size() - 1);
	return (uint32_t)(variants.size() - 1);
}

/// Uniform stuff
Shader::BindingsCache Shader::bindings_cache;

void Shader::setup_builtins()
{
	bindings_cache.clear();

	// Uniform
	bindings_cache.add_builtin(BindingsCache::Uniform, "PER_OBJECT");
	// Storage
	bindings_cache.add_builtin(BindingsCache::Storage, "GLOBAL");
}

void Shader::load_uniforms(Program *prgm)
{
	unsigned program = prgm->program;
	GLint uniform_count, uniform_block_count, ssbo_count;

	// Find max uniform name length
	GLint max_len, temp_len;
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_len));
	glCheck(glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_MAX_NAME_LENGTH, &temp_len));
	if (temp_len > max_len) max_len = temp_len;
	glCheck(glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &temp_len));
	if (temp_len > max_len) max_len = temp_len;
	char *temp_name = new char[max_len];

	// UBO
	glCheck(glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &uniform_block_count));
	for (GLint i = 0; i < uniform_block_count; ++i)
	{
		glCheck(glGetProgramResourceName(program, GL_UNIFORM_BLOCK, i, max_len, &temp_len, temp_name));

		std::string var_name(temp_name, temp_len);
		auto slot = bindings_cache.get_slot(BindingsCache::Uniform, var_name);
		if (slot == -1) continue;

		GLuint index = glGetProgramResourceIndex(program, GL_UNIFORM_BLOCK, temp_name);
		glUniformBlockBinding(program, index, slot);

	}

	// SSBO
	glCheck(glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &ssbo_count));
	for (GLint i = 0; i < ssbo_count; ++i)
	{
		glCheck(glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, max_len, &temp_len, temp_name));

		std::string var_name(temp_name, temp_len);
		auto slot = bindings_cache.get_slot(BindingsCache::Storage, var_name);
		if (slot == -1) continue;

		GLuint index = glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, temp_name);
		glShaderStorageBlockBinding(program, index, slot);
	}

	// Uniforms
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count));
	for (GLint i = 0; i < uniform_count; ++i)
	{
		Program::Uniform u;

		glCheck(glGetActiveUniform(program, i, max_len, &temp_len, &u.num, &u.type, temp_name));
		u.location = glGetUniformLocation(program, temp_name);

		if (u.location == -1)
			continue;

		// Remove '[...]' from variable name if its an array
		if (u.num != 1)
			temp_len = (int)(strchr(temp_name, '[') - temp_name);

		u.size = uniform_type_size.at(u.type);

		std::string var_name(temp_name, temp_len);
		auto it = uniforms_names.find(var_name);
		if (it != uniforms_names.end())
		{
			// Uniform is already defined by an other variant
			u.offset = it->second;
		}
		else
		{
			u.offset = uniform_offset;
			uniform_offset += (size_t)u.size * u.num;

			uniforms_names.emplace(std::move(var_name), u.offset);
		}

		prgm->uniforms.push_back(u);
	}

	delete[] temp_name;
}
