#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <glm/gtc/integer.hpp>

#include "Graphics/Shaders/Shader.inl"
#include "Graphics/Shaders/Program.h"
#include "Graphics/Textures/Texture.h"
#include "Graphics/GLDriver.h"

#include "Utility/Error.h"
#include "Memory/Memory.h"
#include "IO/json.hpp"
#include "IO/URI.h"

using json = nlohmann::json;

Shader *Shader::_standard = NULL, *Shader::_debug = NULL;

static const std::unordered_map<std::string, RenderPass::Type> pass_type = {
	{"shadow",	RenderPass::Shadow},
	{"depth",	RenderPass::Depth},
	{"forward",	RenderPass::Forward},
};

static const std::unordered_map<GLuint, uint8_t> uniform_type_size = {
	{GL_FLOAT,	sizeof(float)},
	{GL_FLOAT_VEC2, sizeof(vec2)},
	{GL_FLOAT_VEC3, sizeof(vec3)},
	{GL_FLOAT_VEC4, sizeof(vec4)},
	{GL_FLOAT_MAT3, sizeof(mat3)},
	{GL_FLOAT_MAT4, sizeof(mat4)},
	{GL_SAMPLER_2D, sizeof(Texture)},
};

static const std::unordered_map<GLuint, const char*> uniform_type_names = {
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



static std::unordered_map<std::string, Shader*> shaders;

Shader* Shader::import(const char *URI)
{
	uri_t uri;
	if (!uri.parse(URI))
		return nullptr;

	if (uri.on_disk)
	{
		std::string path = "Assets/" + uri.path;
		auto it = shaders.find(path);
		if (it != shaders.end())
			return it->second;

		Shader *s = new Shader();
		if (!s->load(path))
		{
			delete s;
			return nullptr;
		}
		shaders.emplace(path, s);
		return s;
	}
	else
	{
		return nullptr;
	}
}

void Shader::clear()
{
	for (auto &entry: shaders)
		delete entry.second;
}

Shader::~Shader()
{
	std::unordered_set<Program*> freed;

	for (Variant &variant : variants)
	{
		for (int i(0); i < RenderPass::Count; i++)
		{
			if (freed.find(variant.passes[i]) == freed.end())
				continue;
			freed.emplace(variant.passes[i]);
			delete variant.passes[i];
		}
	}
}

// Variant stuff
bool Shader::load(const std::string &path)
{
	// Clear state
	for (int i(0); i < RenderPass::Count; i++)
		passes[i].exists = false;
	variants.clear();
	variant_idx.clear();
	macros.clear();
	next_bit = 0;
	uniform_offset = 0;


	// Parse file
	std::ifstream file(path);
	if (!file)
	{
		Error::add(Error::FILE_NOT_FOUND, "Shader::load() -> " + path);
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
			Error::add(Error::USER, "Shader::load() -> unknow pass type '" + pass_name + "'. Skipping...");
			continue;
		}

		passes[type->second].exists = true;
		passes[type->second].mask = 0;

		// Parser variants
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

	// Generate builtins declarations
	std::string builtins_decl;
	for (auto &var : builtins_names)
	{
		unsigned *type = (GLuint*)(builtins.data() + var.second + 1);
		builtins_decl += "uniform ";
		builtins_decl += uniform_type_names.at(*type);
		builtins_decl += " ";
		builtins_decl += var.first;
		builtins_decl += ";\n";
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
			variant.passes[i] = new Program(passes[i].sources, (RenderPass::Type)i, defines.c_str(), builtins_decl.c_str());
			load_uniforms(variant.passes[i]);
		}
	}

	variant_idx.emplace(hash, variants.size() - 1);
	return (uint32_t)(variants.size() - 1);
}

// Uniform stuff
std::vector<uint8_t> Shader::builtins;
std::unordered_map<std::string, size_t> Shader::builtins_names;

void Shader::setup_builtins()
{
	builtins_names.clear();
	builtins.clear();

	// Camera
	add_builtin("MATRIX_VP", GL_FLOAT_MAT4);
	add_builtin("VIEW_POS", GL_FLOAT_VEC3);

	// Light
	add_builtin("MATRIX_LIGHT", GL_FLOAT_MAT4);
	add_builtin("LIGHT_DIR", GL_FLOAT_VEC3);
	add_builtin("LIGHT_COLOR", GL_FLOAT_VEC3);
	add_builtin("SHADOW_MAP", GL_SAMPLER_2D);

	// Model
	add_builtin("MATRIX_M", GL_FLOAT_MAT4);
	add_builtin("MATRIX_N", GL_FLOAT_MAT4);


	Shader::_standard = Shader::import("assets://Shaders/standard.json");
	Shader::_debug = Shader::import("assets://Shaders/debug.json");

}

void Shader::add_builtin(std::string name, unsigned type)
{
	size_t offset = builtins.size();
	uint8_t size = uniform_type_size.at(type);

	builtins_names[name] = offset;

	/*
	 * Builtin structure:
	 * - uint8_t update_idx;
	 * - GLuint type;
	 * - uint8_t data[size];
	 */
	builtins.resize(offset + sizeof(uint8_t) + sizeof(GLuint) + size);

	uint8_t *start = builtins.data() + offset;
	*start = 0; start += sizeof(uint8_t);
	*(GLuint *)start = type;
}

void Shader::load_uniforms(Program *prgm)
{
	unsigned program = prgm->program;

	GLint name_len, real_len;
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &name_len));
	char *temp_name = new char[name_len];

	GLint uniform_count = 0;
	glCheck(glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count));

	for (GLint i = 0; i < uniform_count; ++i)
	{
		Program::Uniform u;

		GLint num;
		glCheck(glGetActiveUniform(program, i, name_len, &real_len, &num, &u.type, temp_name));
		u.location = glGetUniformLocation(program, temp_name);

		if (u.location == -1)
			continue;

		// Remove '[...]' from variable name if its an array
		if (num != 1)
			real_len = (int)(strchr(temp_name, '[') - temp_name);

		std::string var_name(temp_name, real_len);
		auto it = builtins_names.find(var_name);

		// If builtin or not
		if (it != builtins_names.end())
			prgm->builtins.push_back({u.location, 0, it->second});
		else
		{
			u.size = uniform_type_size.at(u.type);
			u.num = num;

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
	}

	delete[] temp_name;
}

Program *Shader::update_builtins(uint32_t variant_idx, RenderPass::Type pass)
{
	Program *prgm = variants[variant_idx].passes[pass];
	GL::UseProgram(prgm->program);

	for (Program::Builtin &var : prgm->builtins)
	{
		uint8_t *b = builtins.data() + var.offset;

		uint8_t update_idx = *b;
		if (var.update_idx == update_idx)
			continue;
		var.update_idx = update_idx;

		b += sizeof(uint8_t);
		GLuint type = *(GLuint*)b;

		b += sizeof(GLuint);

		set_uniform(var.location, type, 1, b);
	}

	return prgm;
}
