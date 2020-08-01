#pragma once

#include <stdint.h>

#include <string>
#include <vector>

#include "Render/RenderPass.h"

struct Program
{
	Program(const struct ShaderSources &sources, RenderPass::Type pass, const char *defines);
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

	std::vector<Uniform> uniforms;
};


