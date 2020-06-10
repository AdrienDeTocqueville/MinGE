#pragma once

#include <stdint.h>

#include "Core/Entity.h"
#include "Core/System.h"

#include "Graphics/GLDriver.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Shaders/Material.h"

#include "Math/glm.h"
#include "Structures/Bounds.h"
#include "Structures/SOA.h"
#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"
#include "Structures/EntityMapper.h"

// TODO: like seriously
using Camera = uint32_t;
using Renderer = uint32_t;
using Light = uint32_t;

struct TransformSystem;

struct GraphicsSystem
{
	GraphicsSystem(TransformSystem *world);
	~GraphicsSystem();

	Camera add_camera(Entity entity, float FOV = 70.0f, float zNear = 0.1f, float zFar = 1000.0f,
		bool orthographic = false, vec4 viewport = vec4(0.0f,0.0f,1.0f,1.0f),
		vec3 clear_color = vec3(0.0f), unsigned clear_flags = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Renderer add_renderer(Entity entity, Mesh mesh);

	Light add_point_light(Entity entity, vec3 color = vec3(150.0f / 255.0f));

	// Cameras
	struct camera_t
	{
		mat4 projection;
		Frustum frustum;
		float zNear, zFar;
		vec3 center_point;

		float fov;
		Entity entity;
		vec4 ss_viewport;	// screen space (between 0 and 1)
		bool ortho;
	};

	soa_t<camera_t, struct camera_data_t> cameras; // TODO: don't need a whole page

	// Renderers
	struct renderer_t
	{
		uint32_t first_submesh, last_submesh;
		Entity entity;
		Mesh mesh;
	};

	soa_t<renderer_t> renderers;
	array_list_t<struct submesh_data_t> submeshes;

	// Lights
	struct point_light_t
	{
		vec3 color;
		Entity entity;
	};

	std::vector<point_light_t> point_lights;

	// System data
	entity_mapper_t<3> indices;

	uint32_t cmd_buffer;

	uint32_t prev_index_count;
	uint32_t prev_key_count;
	uint32_t prev_renderer_count;

	uint32_t *draw_order_indices;
	uint64_t *renderer_keys;
	mat4 *matrices;

	TransformSystem *transforms;

	static const system_type_t type;


	// Serialization
	GraphicsSystem(const SerializationContext &ctx);
	void save(SerializationContext &ctx) const;
};
