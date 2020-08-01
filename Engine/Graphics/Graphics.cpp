#include <algorithm>

#include "Profiler/profiler.h"
#include "Core/Engine.h"
#include "IO/Input.h"

#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"
#include "Render/Debug.h"
#include "Render/Shader/Shader.inl"

#include "Graphics/Graphics.h"
#include "Graphics/CommandKey.h"
#include "Transform/Transform.h"

GraphicsSystem::GraphicsSystem(TransformSystem *world):
	indices_capacity(0), keys_capacity(0),
	draw_order_indices(NULL), renderer_keys(NULL),
	transforms(world)
{
	RenderEngine::add_buffer(&cmd_buffer);

	global.buffer = GL::GenBuffer();
	objects.buffer = GL::GenBuffer();
}

GraphicsSystem::~GraphicsSystem()
{
	for (uint32_t i = 1; i <= cameras.size; i++)
	{
		camera_t *cam = cameras.get<0>() + i;

		GL::DeleteFramebuffer(cam->fbo_depth);
		GL::DeleteFramebuffer(cam->fbo_forward);

		cam->depth_buffer.destroy();
		cam->depth_texture.destroy();
		cam->color_texture.destroy();
	}

	RenderEngine::remove_buffer(&cmd_buffer);

	free(draw_order_indices);
	free(renderer_keys);
}


// Must match builtin.glsl
struct camera_data_t
{
	mat4 view_proj;
	vec3 position;
	float padding;
};
struct light_data_t
{
	vec3 position;
	float radius;
	vec3 color;
	float padding;
};

static void init_indices(uint32_t *__restrict indices, const GraphicsSystem::renderer_t *renderers, uint32_t count)
{
	for (uint32_t j = 0; j < count; j++)
	{
		for (int s = renderers->first_submesh; s != renderers->last_submesh; s++)
			*indices++ = s;
		renderers++;
	}
}

static void update(GraphicsSystem *self)
{
	{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "read_lock");
	Engine::read_lock(self);
	}

	auto renderer_count = self->renderers.size;
	auto *renderers = self->renderers.get<0>() + 1;
	auto camera_count = self->cameras.size;
	auto light_count = self->point_lights.size;
	auto &submeshes = self->submeshes;

	const unsigned object_size = mem::align(sizeof(mat4) * 2, GL::uniform_offset_alignment);
	const uint32_t global_size = sizeof(camera_data_t) + light_count * sizeof(light_data_t);

	{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "set_counters");
	MICROPROFILE_COUNTER_SET("GRAPHICS/renderers", renderer_count);
	MICROPROFILE_COUNTER_SET("GRAPHICS/cameras", camera_count);
	MICROPROFILE_COUNTER_SET("GRAPHICS/lights", light_count);
	}

	/// Resize persistent alloc if needed
	{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "realloc");

	uint32_t new_index_count = submeshes.count * camera_count;
	if (self->indices_capacity < new_index_count)
	{
		self->indices_capacity = mem::next_power_of_two(new_index_count);
		self->draw_order_indices = (uint32_t*)realloc(self->draw_order_indices, self->indices_capacity * sizeof(uint32_t));
		goto recompute_indices;
	}
	if (self->objects.capacity & (-1))
	{
		recompute_indices:
		self->objects.capacity &= ~(-1);
		init_indices(self->draw_order_indices, renderers, renderer_count);
		for (uint32_t i = 1; i < camera_count; i++)
			memcpy(self->draw_order_indices + i * submeshes.count, self->draw_order_indices, submeshes.count * sizeof(uint32_t));
	}

	if (self->keys_capacity < submeshes.size * camera_count)
	{
		self->keys_capacity = mem::next_power_of_two(submeshes.size * camera_count);
		self->renderer_keys = (uint64_t*)realloc(self->renderer_keys, self->keys_capacity * sizeof(uint64_t));
	}

	if (self->global.capacity < global_size)
	{
		self->global.capacity = mem::next_power_of_two(global_size);
		self->global.data = (mat4*)realloc(self->global.data, self->global.capacity * camera_count);
	}
	if (self->objects.capacity < renderer_count)
	{
		self->objects.capacity = mem::next_power_of_two(renderer_count);
		self->objects.data = (mat4*)realloc(self->objects.data, self->objects.capacity * object_size);
	}
	}

	auto *cameras = self->cameras.get<0>() + 1;
	auto *point_lights = self->point_lights.get<0>() + 1;
	uint8_t *matrices = (uint8_t*)self->objects.data;
	uint8_t *global = (uint8_t*)self->global.data;

	/// Read transform data
	{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "sync_transform");

		auto transforms = self->transforms;
		Engine::read_lock(transforms);

		// TODO: transforms don't need to be fetched every time
		for (uint32_t i = 0; i < renderer_count; i++)
		{
			Transform tr = transforms->get(renderers[i].entity);
			auto obj = (mat4*)(matrices + i * object_size);
			obj[0] = tr.world_matrix();
			obj[1] = tr.normal_matrix();
		}
		for (uint32_t i = 0; i < camera_count; i++)
		{
			camera_data_t *camera_data = (camera_data_t*)(global + i * global_size);
			Transform tr = transforms->get(cameras[i].entity);
			vec3 pos = tr.position();

			cameras[i].center_point = tr.vec_to_world(vec3(1, 0, 0)) + pos;
			cameras[i].up_vector = tr.vec_to_world(vec3(0, 0, 1));
			camera_data->position = pos;
		}

		light_data_t *lights_data = (light_data_t*)(global + sizeof(camera_data_t));
		for (uint32_t i = 0; i < light_count; i++)
		{
			Transform tr = transforms->get(point_lights[i].entity);
			lights_data[i].position = tr.position();
			lights_data[i].radius = point_lights[i].radius;
			lights_data[i].color = point_lights[i].color;
		}

		Engine::read_unlock(transforms);

		for (uint32_t i = 1; i < camera_count; i++)
			memcpy(lights_data + i * global_size, lights_data, light_count * sizeof(light_data_t));
	}

	/// Build command buffer
	cmd_buffer_t &cmd = self->cmd_buffer;
	cmd.set_uniform_data(self->objects.buffer, matrices, object_size * renderer_count);
	cmd.set_storage_data(self->global.buffer, global, global_size * camera_count);

	Sphere sphere;
	for (uint32_t i = 0; i < camera_count; i++)
	{
		MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "camera_setup");

		uint32_t cmd_count = submeshes.count;
		uint64_t *renderer_keys = self->renderer_keys + i * submeshes.size;
		GraphicsSystem::camera_t *cam = cameras + i;
		camera_data_t *camera_data = (camera_data_t*)(global + i * global_size);
		float scale = 1.0f / (cam->far_plane - cam->near_plane);

		/// Compute new VP

		const mat4 view_matrix = glm::lookAt(camera_data->position, cam->center_point, cam->up_vector);
		simd_mul(camera_data->view_proj, cam->projection, view_matrix);

		/// Frustum culling
		{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "frustum_culling");

		cam->frustum.init(camera_data->view_proj);

		for (uint32_t j = 0; j < renderer_count; j++)
		{
			if (renderers[j].mesh == Mesh::none) continue;

			// TODO: don't recompute sphere for each camera ?
			sphere.init(Mesh::meshes.get<3>()[renderers[j].mesh.id()]);
			sphere.transform(*(mat4*)(matrices + j * object_size));

			// TODO: SIMDify
			if (Bounds::collide(cam->frustum, sphere))
			{
				union float_int_t {
					float_int_t(int32_t x): as_int(x) {}
					float as_float;
					int32_t as_int;
				};
				static float_int_t max_dist(0x3FFFFFFF); // last float before 2.0f

				vec4 p = cam->frustum.planes[Frustum::Near];
				vec3 c = sphere.center;
				float distance = p.x * c.x + p.y * c.y + p.z * c.z + p.w;
				// floats between 1.0f and 2.0f only use the first 23 bits
				// and can be compared as uint
				distance = glm::clamp(distance * scale + 1.0f, 1.0f, max_dist.as_float);

				for (int s = renderers[j].first_submesh; s != renderers[j].last_submesh; s++)
				{
					submesh_data_t *data = &submeshes[s];
					renderer_keys[s] = CommandKey::encode(data->material, 0, distance); // TODO
				}
			}
			else
			{
				// frustum culled
				uint32_t first = renderers[j].first_submesh, last = renderers[j].last_submesh;
				memset(renderer_keys + first, ~0, (last - first) * sizeof(uint64_t));
				cmd_count -= last - first;
			}
		}
		}

		uint32_t *draw_order_indices = self->draw_order_indices + i * submeshes.count;

		/// Sort renderers
		{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "sorting");

		std::sort(draw_order_indices, draw_order_indices + self->submeshes.count,
			[renderer_keys](uint32_t a, uint32_t b) { return renderer_keys[a] < renderer_keys[b]; }
		);
		}

		/// Submit to backend
		{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "submit");

		cmd.setup_camera(cam->screen_size, self->global.buffer, i * global_size, global_size);

		cmd.set_framebuffer(cam->fbo_depth, vec4(1.0f), true);
		cmd.draw_batch(submeshes.data, draw_order_indices, self->objects.buffer, RenderPass::Depth, cmd_count);

		cmd.set_framebuffer(cam->fbo_forward, cam->clear_color, false);
		cmd.draw_batch(submeshes.data, draw_order_indices, self->objects.buffer, RenderPass::Forward, cmd_count);
		}
	}

	if (camera_count)
	{
		// Use first camera for debug draw
		Debug::cmd().setup_camera(cameras->screen_size, self->global.buffer, 0, global_size);
	}

	Engine::read_unlock(self);
}


static void on_destroy_entity(GraphicsSystem *self, Entity e)
{
	assert(false && "todo");
}

static void on_resize_window(GraphicsSystem *self)
{
	for (uint32_t i = 1; i <= self->cameras.size; i++)
		self->resize_rt(i);
}

const system_type_t GraphicsSystem::type = []() {
	system_type_t t = INIT_SYSTEM_TYPE(GraphicsSystem);

	t.destroy = [](void *system) { ((GraphicsSystem*)system)->~GraphicsSystem(); };
	t.update = (decltype(t.update))update;

	t.on_destroy_entity = (decltype(t.on_destroy_entity))on_destroy_entity;
	t.on_resize_window = (decltype(t.on_resize_window))on_resize_window;

	t.save = [](void *system, SerializationContext &ctx) { ((GraphicsSystem*)system)->save(ctx); };
	t.load = [](void *system, const SerializationContext &ctx) { new(system) GraphicsSystem(ctx); };
	return t;
}();
