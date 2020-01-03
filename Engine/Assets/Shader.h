#pragma once

#include <cstdint>

#include <string>
#include <vector>
#include <unordered_map>

#include "Renderer/structs.h"


class Shader
{
	friend class Material;
	friend class GraphicEngine;

public:
	static Shader *get(const std::string &name);

	static inline size_t getBuiltinLocation(const std::string &name);

	template <typename T>
	static inline void setBuiltin(std::string name, T value);
	template <typename T>
	static inline void setBuiltin(size_t location, T value);

private:
	~Shader();

	static void clear();
	static void setupBuiltins();
	static void addBuiltin(std::string name, unsigned type);

	bool load(const std::string &file);
	void load_uniforms(struct Program *prgm);

	uint32_t get_variant(uint32_t hash);
	Program *updateBuiltins(uint32_t variant_idx, RenderPass::Type pass);


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
};

template<>
inline void Shader::setBuiltin(size_t location, const class Texture *value);
