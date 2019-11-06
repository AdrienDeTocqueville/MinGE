#pragma once

#include "Renderer/Commands.h"
#include "Assets/Material.h"
#include "Assets/Mesh.h"

struct CommandBucket
{
	template<typename T>
	T *add(Mesh *mesh, int submesh, Material *material, float depth, const mat4 &toWorld)
	{
		return nullptr;
	}

	void sort();
	void submit();
	void clear();


	// View state
	vec4 viewport;
	unsigned fbo;
	mat4 vp;
	vec4 clearColor;
	unsigned clearFlags;

	RenderPass pass;
};
