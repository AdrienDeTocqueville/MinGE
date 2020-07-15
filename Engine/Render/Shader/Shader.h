#pragma once

#include <stdint.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "Render/RenderPass.h"

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
public:
	friend struct Material;
	friend struct material_t;
	friend struct RenderEngine;

	struct Variant
	{
		struct Program *passes[RenderPass::Count];
	};

	struct Macro
	{
		uint32_t mask;
		uint32_t id;
	};

	static Shader *load(const char *URI);

	static inline size_t get_builtin_location(const std::string &name);

	template <typename T>
	static inline void set_builtin(const std::string &name, const T &value);
	template <typename T>
	static inline void set_builtin(size_t location, const T &value);

	const char *URI;

	std::unordered_map<std::string, Macro> macros;
	std::vector<Variant> variants;
	std::unordered_map<std::string, size_t> uniforms_names;

private:
	~Shader();

	static void clear();
	static void setup_builtins();
	static void add_builtin(std::string name, unsigned type);

	bool load_json(const char *path);
	void load_uniforms(struct Program *prgm);
	Shader *reload();

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

	std::unordered_map<uint32_t, uint32_t> variant_idx;

	uint32_t next_bit;
	size_t uniform_offset;

	const char *label;
	int label_len;

	static std::vector<uint8_t> builtins; // Contains < update_idx | type | data > sequenced for each builtin
	static std::unordered_map<std::string, size_t> builtins_names;
};
