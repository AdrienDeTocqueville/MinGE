#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

#include "Core/Entity.h"
#include "Core/System.h"

#include "Graphics/GLDriver.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Shaders/Material.h"

#include "Math/glm.h"
#include "Structures/AABB.h"
#include "Structures/MultiArray.h"

using Camera = uint32_t;
using Renderer = uint32_t;

struct TransformSystem;

struct GraphicsSystem
{
	GraphicsSystem(TransformSystem *world);

	Camera add_camera(Entity entity, float FOV = 60.0f, float zNear = 0.1f, float zFar = 1000.0f,
		bool orthographic = false, vec4 viewport = vec4(0.0f,0.0f,1.0f,1.0f),
		vec3 clear_color = vec3(0.0f), unsigned clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Renderer add_renderer(Entity entity, Mesh mesh);

	struct camera_t
	{
		float fov, zNear, zFar;

		vec3 clear_color;
		unsigned clear_flags;

		bool ortho;
		Entity entity;

		ivec4 viewport;
		vec4 ss_viewport;	// screen space (between 0 and 1)

		mat4 projection;
		vec3 position;
		Frustum frustum;
	};

	std::unordered_map<uint32_t, uint32_t> indices_cameras;
	std::vector<camera_t> cameras;

	struct renderer_t
	{
		Mesh mesh;
		submeshes_t submeshes;
		uint32_t first_material;
	};

	std::unordered_map<uint32_t, uint32_t> indices_renderers;
	std::vector<Entity> renderers_entities;
	std::vector<renderer_t> renderers;

	TransformSystem *transforms;

	static const system_type_t type;
};


namespace RenderEngine
{
	void init();
}
