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
#include "Structures/Bounds.h"
#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"

using Camera = uint32_t;
using Renderer = uint32_t;

struct TransformSystem;

struct GraphicsSystem
{
	GraphicsSystem(TransformSystem *world);
	~GraphicsSystem();

	Camera add_camera(Entity entity, float FOV = 70.0f, float zNear = 0.1f, float zFar = 1000.0f,
		bool orthographic = false, vec4 viewport = vec4(0.0f,0.0f,1.0f,1.0f),
		vec3 clear_color = vec3(0.0f), unsigned clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Renderer add_renderer(Entity entity, Mesh mesh);

	// Cameras
	struct camera_t
	{
		float fov, zNear, zFar;

		vec3 clear_color;
		unsigned clear_flags;

		bool ortho;
		Entity entity;

		ivec4 viewport;
		vec4 ss_viewport;	// screen space (between 0 and 1)

		mat4 projection, vp;
		vec3 position;

		Frustum frustum;
		uint32_t *draw_order_indices;
		uint64_t *renderer_keys;
	};

	std::unordered_map<uint32_t, uint32_t> indices_cameras;
	std::vector<camera_t> cameras;

	// Renderers
	struct renderer_t
	{
		uint32_t first_submesh, last_submesh;
		Entity entity;
		Mesh mesh;
	};

	std::unordered_map<uint32_t, uint32_t> indices_renderers;
	std::vector<renderer_t> renderers;
	array_list_t<struct submesh_data_t> submeshes;


	// System data
	uint32_t prev_submesh_count;
	uint32_t prev_submesh_alloc;
	uint32_t prev_renderer_count;
	mat4 *matrices;

	TransformSystem *transforms;

	static const system_type_t type;
};
