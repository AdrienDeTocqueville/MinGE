#include <algorithm>

#include "Profiler/profiler.h"
#include "Core/Engine.h"

#include "Render/RenderEngine.h"
#include "Render/CommandBuffer.h"
#include "Render/Debug.h"
#include "Render/Shaders/Shader.inl"

#include "Graphics/Graphics.h"
#include "Graphics/CommandKey.h"
#include "Transform/Transform.h"

GraphicsSystem::GraphicsSystem(TransformSystem *world):
	prev_index_count(0), prev_key_count(0), prev_renderer_count(0),
	draw_order_indices(NULL), renderer_keys(NULL), matrices(NULL),
	transforms(world)
{
	cmd_buffer = RenderEngine::create_cmd_buffer();
}

GraphicsSystem::~GraphicsSystem()
{
	RenderEngine::destroy_cmd_buffer(cmd_buffer);

	free(draw_order_indices);
	free(renderer_keys);
	free(matrices);
}

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
	Engine::read_lock(self);
	material_t::bound = nullptr;

	auto renderer_count = self->renderers.size;
	auto *renderers = self->renderers.get<0>() + 1;
	auto camera_count = self->cameras.size;
	auto &submeshes = self->submeshes;

	MICROPROFILE_COUNTER_SET("GRAPHICS/renderers", renderer_count);
	MICROPROFILE_COUNTER_SET("GRAPHICS/cameras", camera_count);
	MICROPROFILE_COUNTER_SET("GRAPHICS/point_lights", self->point_lights.size());

	/// Resize persistent alloc if needed
	{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "realloc");

	uint32_t new_index_count = submeshes.count * camera_count;
	if (self->prev_index_count < new_index_count)
	{
		self->prev_index_count = mem::next_power_of_two(new_index_count);
		self->draw_order_indices = (uint32_t*)realloc(self->draw_order_indices, self->prev_index_count * sizeof(uint32_t));
		goto recompute_indices;
	}
	if (self->prev_renderer_count != renderer_count)
	{
		recompute_indices:
		init_indices(self->draw_order_indices, renderers, renderer_count);
		for (uint32_t i = 1; i < camera_count; i++)
			memcpy(self->draw_order_indices + i * submeshes.count, self->draw_order_indices, submeshes.count * sizeof(uint32_t));
	}

	if (self->prev_key_count < submeshes.size * camera_count)
	{
		self->prev_key_count = mem::next_power_of_two(submeshes.size * camera_count);
		self->renderer_keys = (uint64_t*)realloc(self->renderer_keys, self->prev_key_count * sizeof(uint64_t));
	}

	if (self->prev_renderer_count < renderer_count)
	{
		self->prev_renderer_count = mem::next_power_of_two(renderer_count);
		self->matrices = (mat4*)realloc(self->matrices, self->prev_renderer_count * sizeof(mat4));
	}
	}

	auto *cameras = self->cameras.get<0>() + 1;
	auto *camera_datas = self->cameras.get<1>() + 1;
	mat4 *matrices = self->matrices;

	/// Read transform data
	{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "sync_transform");

		auto transforms = self->transforms;
		Engine::read_lock(transforms);

		// TODO: transforms don't need to be fetched every time
		for (uint32_t i = 0; i < renderer_count; i++)
		{
			Transform tr = transforms->get(renderers[i].entity);
			matrices[i] = tr.world_matrix();
		}
		for (uint32_t i = 0; i < camera_count; i++)
		{
			Transform tr = transforms->get(cameras[i].entity);
			vec3 pos = tr.position();

			cameras[i].center_point = tr.vec_to_world(vec3(1, 0, 0)) + pos;
			cameras[i].up_vector = tr.vec_to_world(vec3(0, 0, 1));
			camera_datas[i].position = pos;
		}
		for (uint32_t i = 0; i < self->point_lights.size(); i++)
		{
			Transform tr = transforms->get(self->point_lights[i].entity);
			Shader::set_builtin("LIGHT_DIR", tr.position());
			Shader::set_builtin("LIGHT_COLOR", self->point_lights[i].color);
		}

		Engine::read_unlock(transforms);
	}

	/// Build command buffers

	Sphere sphere;
	auto &cmd = RenderEngine::get_cmd_buffer(self->cmd_buffer);
	for (uint32_t i = 0; i < camera_count; i++)
	{
		MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "camera_setup");

		uint32_t cmd_count = renderer_count;
		uint64_t *renderer_keys = self->renderer_keys + i * submeshes.size;
		GraphicsSystem::camera_t *cam = cameras + i;
		camera_data_t *cam_data = camera_datas + i;
		float scale = 1.0f / (cam->zFar - cam->zNear);

		/// Compute new VP

		const mat4 view_matrix = glm::lookAt(cam_data->position, cam->center_point, cam->up_vector);
		simd_mul(cam_data->view_proj, cam->projection, view_matrix);

		/// Frustum culling
		{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "frustum_culling");

		cam->frustum.init(cam_data->view_proj);

		for (uint32_t j = 0; j < renderer_count; j++)
		{
			// TODO: don't recompute sphere for each camera
			sphere.init(Mesh::meshes.get<3>()[renderers[j].mesh.id()]);
			sphere.transform(matrices[j]);

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

		if (i != 0) // some debug draw
		{
			MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "debug");

			vec3 color(1, 0, 0);
			for (uint32_t c = 0; c < cmd_count; c++)
			{
				uint32_t s = draw_order_indices[c];
				submesh_data_t *data = &self->submeshes[s];

				sphere.init(Mesh::meshes.get<3>()[renderers[data->renderer].mesh.id()]);
				sphere.transform(matrices[data->renderer]);

				Debug::sphere(sphere, color);
				color += vec3(0, 1, 0);
			}
			continue;
		}

		/// Submit to backend
		{ MICROPROFILE_SCOPEI("GRAPHICS_SYSTEM", "submit");

		cmd.setup_camera(cam_data);
		cmd.draw_batch(submeshes.data, matrices, draw_order_indices, RenderPass::Forward, cmd_count);
		}
	}
	Engine::read_unlock(self);
}


const system_type_t GraphicsSystem::type = []() {
	system_type_t t{};
	t.name = "GraphicsSystem";
	t.size = sizeof(GraphicsSystem);

	t.destroy = [](void *system) { ((GraphicsSystem*)system)->~GraphicsSystem(); };
	t.update = (void(*)(void*))update;
	t.on_destroy_entity = NULL;
	t.save = [](void *system, SerializationContext &ctx) { ((GraphicsSystem*)system)->save(ctx); };
	t.load = [](void *system, const SerializationContext &ctx) { new(system) GraphicsSystem(ctx); };
	return t;
}();
