#pragma once

#include <cstdint>

#include <string>
#include <vector>
#include <unordered_map>

#include "Graphics/RenderPass.h"

struct ShaderSources
{
	std::string vertex;
	std::string tess_ctrl;
	std::string tess_eval;
	std::string geometry;
	std::string fragment;
};

class Shader
{
	friend struct Material;
	friend struct material_t;
	friend struct RenderEngine;

public:
	static Shader *import(const char *URI);
	static Shader *standard() { return _standard; }
	static Shader *debug() { return _debug; }

	static inline size_t get_builtin_location(const std::string &name);

	template <typename T>
	static inline void set_builtin(std::string name, T value);
	template <typename T>
	static inline void set_builtin(size_t location, T value);

private:
	~Shader();

	static void clear();
	static void setup_builtins();
	static void add_builtin(std::string name, unsigned type);

	bool load(const std::string &path);
	void load_uniforms(struct Program *prgm);

	uint32_t get_variant(uint32_t hash);
	Program *update_builtins(uint32_t variant_idx, RenderPass::Type pass);


	// Variant related stuff
	struct Pass
	{
		bool exists;
		uint32_t mask;
		ShaderSources sources;
	};
	Pass passes[RenderPass::Count];

	struct Variant
	{
		struct Program *passes[RenderPass::Count];
	};
	std::vector<Variant> variants;
	std::unordered_map<uint32_t, uint32_t> variant_idx;

	struct Macro
	{
		uint32_t mask;
		uint32_t id;
	};
	std::unordered_map<std::string, Macro> macros;
	uint32_t next_bit;

	// Uniforms related stuff
	size_t uniform_offset;
	std::unordered_map<std::string, size_t> uniforms_names;

	static std::vector<uint8_t> builtins; // Contains < update_idx | type | data > sequenced for each builtin
	static std::unordered_map<std::string, size_t> builtins_names;

	static Shader *_standard, *_debug;
};
