#pragma once

#include <string>

struct RenderPass
{
	enum Type {
		Shadow,
		Forward,
		Additive,
		Skybox,
		Count
	};
};

struct ShaderSources
{
	std::string vertex;
	std::string tess_ctrl;
	std::string tess_eval;
	std::string geometry;
	std::string fragment;
};