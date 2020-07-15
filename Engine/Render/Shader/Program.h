#pragma once

#include <stdint.h>

#include <string>
#include <vector>

#include "Render/RenderPass.h"

struct Program
{
	Program(const struct ShaderSources &sources, RenderPass::Type pass, const char *defines, const char *builtins);
	~Program();
	void label(const char *name, size_t len);

	unsigned program;

	struct Uniform
	{
		int location;
		unsigned type, size;
		int num;
		size_t offset; // offset in the array 'uniforms' of each Material
	};
	struct Builtin
	{
		int location;
		uint8_t update_idx;
		size_t offset; // offset in the static array 'builtins'
	};

	std::vector<Uniform> uniforms;
	std::vector<Builtin> builtins;
};


