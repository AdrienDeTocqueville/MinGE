#pragma once

#include <stdint.h>

#include "Core/Entity.h"
#include "Core/System.h"

#include "Render/Mesh/Mesh.h"
#include "Render/Shaders/Material.h"
#include "Render/Textures/Texture.h"
#include "Render/Textures/RenderTexture.h"

#include "Math/glm.h"
#include "Structures/Bounds.h"
#include "Structures/SOA.h"
#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"
#include "Structures/EntityMapper.h"

#include "Graphics/GraphicsComponents.h"

struct TransformSystem;

struct GraphicsSystem
{
	GraphicsSystem(TransformSystem *world);
	~GraphicsSystem();

	Camera add_camera(Entity entity, float fov = 70.0f, float near_plane = 0.1f, float far_plane = 1000.0f,
		bool orthographic = false, vec2 size_scale = vec2(1), vec4 clear_color = vec4(0.0f))
	{
		resize_rt(init_camera(entity, fov, near_plane, far_plane, orthographic, size_scale,
			clear_color, Texture::none, Texture::none));
		return {entity.id(), 0, *this};
	}

	Renderer add_renderer(Entity entity, Mesh mesh);

	Light add_point_light(Entity entity, vec3 color = vec3(150.0f / 255.0f));

	bool has_camera(Entity entity)	 const	{ return indices.has<0>(entity); }
	bool has_renderer(Entity entity) const	{ return indices.has<1>(entity); }
	Camera	 get_camera(Entity entity)   { return {entity.id(), 0, *this}; }
	Renderer get_renderer(Entity entity) { return {entity.id(), 0, *this}; }


	// Cameras
	struct camera_t
	{
		vec3 center_point;
		float near_plane;
		vec3 up_vector;
		float far_plane;

		mat4 projection;
		Frustum frustum;
		vec4 clear_color;
		uint32_t fbo_depth, fbo_forward;

		float fov;
		Entity entity;
		vec2 size_scale;

		bool ortho;

		render_texture_t depth_buffer;
		Texture depth_texture;
		Texture color_texture;

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


	void resize_rt(uint32_t i);
	void update_projection(uint32_t i);
	void update_submeshes(uint32_t i, bool remove_previous);


	// Serialization
	GraphicsSystem(const SerializationContext &ctx);
	void save(SerializationContext &ctx) const;

private:
	uint32_t init_camera(Entity entity, float fov, float near_plane, float far_plane, bool orthographic,
		vec2 size_scale, vec4 clear_color, Texture color, Texture depth);
};

#include "Graphics/GraphicsComponents.inl"
