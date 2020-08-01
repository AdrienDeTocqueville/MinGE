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

	static inline uint32_t get_builtin_location(const std::string &name);

	static inline void set_uniform(uint32_t slot, uint32_t buf, uint32_t offset, uint32_t size);
	static inline void set_storage(uint32_t slot, uint32_t buf, uint32_t offset, uint32_t size);

	const char *URI;

	std::unordered_map<std::string, Macro> macros;
	std::vector<Variant> variants;
	std::unordered_map<std::string, size_t> uniforms_names;

private:
	~Shader();

	static void clear();
	static void setup_builtins();

	bool load_json(const char *path);
	void load_uniforms(struct Program *prgm);
	Shader *reload();

	uint32_t get_variant(uint32_t hash);


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


	// Bindings cache system

	struct BindingsCache
	{
		enum Object {Uniform, Storage, COUNT};
		std::unordered_map<std::string, uint32_t> variables;

		void add_builtin(Object type, const std::string &name)
		{
			variables[name] = (uint32_t)bindings[type].size();
			bindings[type].emplace_back();
		}
		uint32_t get_slot(Object type, const std::string &name)
		{
			auto it = variables.find(name);
			if (it == variables.end())
				return -1;
			return it->second;
		}
		void clear()
		{
			variables.clear();
			for (int i = 0; i < Object::COUNT; i++)
				bindings[i].clear();
		}
		inline bool update(Object type, uint32_t slot, uint32_t buf, uint32_t offset, uint32_t size)
		{
			buffer_view_t &bound = bindings[type][slot];
			if (bound.buf == buf && bound.offset == offset && bound.size == size)
				return false;
			bound.buf = buf;
			bound.offset = offset;
			bound.size = size;
			return true;
		}

		struct buffer_view_t
		{
			buffer_view_t(): buf(0) {}
			uint32_t buf;
			uint32_t offset, size;
		};
		std::vector<buffer_view_t> bindings[COUNT];
	};
	static BindingsCache bindings_cache;
};
