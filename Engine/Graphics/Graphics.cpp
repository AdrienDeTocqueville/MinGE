#include "Graphics/Graphics.h"
#include "Graphics/Shaders/Shader.inl"
#include "Graphics/Shaders/Material.inl"
#include "Graphics/RenderEngine.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/CommandKey.h"
#include "Graphics/Debug.h"

#include "Transform/Transform.h"

#include "IO/Input.h"


GraphicsSystem::GraphicsSystem(TransformSystem *world):
	prev_submesh_count(0), submesh_count(0),
	prev_submesh_alloc(0),
	prev_renderer_count(0),
	matrices(NULL), transforms(world)
{
	RenderEngine::alloc_cmd_buffer();
}

GraphicsSystem::~GraphicsSystem()
{
	for (camera_t &c : cameras)
	{
		free(c.draw_order_indices);
		free(c.renderer_keys);
	}
	free(matrices);
}

static uint32_t *compute_indices(uint32_t count, const std::vector<GraphicsSystem::renderer_t> &renderers)
{
	uint32_t *__restrict indices = (uint32_t*)malloc(count * sizeof(uint32_t));
	uint32_t i = 0;

	for (int j = 0; j < renderers.size(); j++)
	{
		for (int s = renderers[j].first_submesh; s != renderers[j].last_submesh; s++)
			indices[i++] = s;
	}
	return indices;
}

Camera GraphicsSystem::add_camera(Entity entity, float FOV, float zNear, float zFar,
	bool orthographic, vec4 viewport, vec3 clear_color, unsigned clear_flags)
{
	assert(entity.id() != 0 && "Invalid entity");
	assert(indices_cameras.find(entity.id()) == indices_cameras.end() && "Entity already has a Camera component");

	uint32_t i = cameras.size();
	cameras.emplace_back(camera_t {});
	indices_cameras[entity.id()] = i;

	camera_t *cam = cameras.data() + i;

	cam->fov	= FOV;
	cam->zNear	= zNear;
	cam->zFar	= zFar;
	cam->clear_color= clear_color;
	cam->clear_flags= clear_flags;
	cam->ortho	= orthographic;
	cam->entity	= entity;
	cam->ss_viewport= viewport;

	cam->draw_order_indices	= compute_indices(submesh_count, renderers);
	cam->renderer_keys	= (uint64_t*)malloc(submeshes.size * sizeof(uint64_t));

	vec2 ws = Input::window_size();
	cam->viewport = ivec4(viewport.x * ws.x,
		viewport.y * ws.y,
		viewport.z * ws.x,
		viewport.w * ws.y
	);

	if (orthographic)
	{
		float half_width = FOV * 0.5f;
		float half_height = half_width * cam->viewport.w / (float)cam->viewport.z;
		cam->projection = ortho(-half_width, half_width, -half_height, half_height, zNear, zFar);
	}
	else
	{
		float aspect_ratio = (float)cam->viewport.z / (float)cam->viewport.w;
		cam->projection = perspective(FOV, aspect_ratio, zNear, zFar);
	}

	return i;
}

Renderer GraphicsSystem::add_renderer(Entity entity, Mesh mesh)
{
	assert(entity.id() != 0 && "Invalid entity");
	assert(indices_renderers.find(entity.id()) == indices_renderers.end() && "Entity already has a Renderer component");

	uint32_t i = renderers.size();
	indices_renderers[entity.id()] = i;

	submeshes_t subs = Mesh::meshes.get<0>()[mesh.id()];

	uint32_t first = submeshes.add(subs.count);
	submesh_count += subs.count;

	for (int s = 0; s < subs.count; s++)
	{
		submeshes[i].submesh = Mesh::submeshes[subs.first + s];
		submeshes[i].material = 2; // TODO: default material
		submeshes[i].renderer = i;
	}

	renderers.emplace_back(renderer_t {first, first + subs.count, entity, mesh});

	return i;
}

static void update(void *system)
{
	material_t::bound = nullptr;
	GraphicsSystem *self = (GraphicsSystem*)system;

	auto &renderers = self->renderers;
	auto &submeshes = self->submeshes;

	/// Resize persistent alloc if needed

	if (self->prev_submesh_count < self->submesh_count)
	{
		self->prev_submesh_count = self->submesh_count;

		for (int i = 0; i < self->cameras.size(); i++)
		{
			GraphicsSystem::camera_t *cam = &self->cameras[i];

			free(cam->draw_order_indices);
			cam->draw_order_indices = compute_indices(self->submesh_count, renderers);
		}
	}
	if (self->prev_submesh_alloc < submeshes.size)
	{
		self->prev_submesh_alloc = submeshes.size;

		for (int i = 0; i < self->cameras.size(); i++)
		{
			GraphicsSystem::camera_t *cam = &self->cameras[i];

			free(cam->renderer_keys);
			cam->renderer_keys = (uint64_t*)malloc(self->prev_submesh_alloc * sizeof(uint64_t));
		}
	}
	if (self->prev_renderer_count < renderers.size())
	{
		self->prev_renderer_count = renderers.size();

		free(self->matrices);
		self->matrices = (mat4*)malloc(self->prev_renderer_count * sizeof(mat4));
	}

	mat4 *matrices = self->matrices;

	/// Read transform data

	struct cam_view_t
	{ vec3 pos, center; };

	// TODO: cache allocation between iterations
	// TODO: mt
	// TODO: use page allocator
	std::vector<cam_view_t> cam_views(self->cameras.size());
	{
		//auto lock = transforms.scoped_read_lock();
		for (int i = 0; i < renderers.size(); i++)
		{
			Transform tr = self->transforms->get(renderers[i].entity);
			matrices[i] = tr.world_matrix();
		}
		for (int i = 0; i < self->cameras.size(); i++)
		{
			Transform tr = self->transforms->get(self->cameras[i].entity);
			cam_views[i].pos = tr.position();
			cam_views[i].center = tr.to_world(vec3(1, 0, 0));
		}
	}

	/// Build command buffers

	Sphere sphere;
	for (int i = 0; i < self->cameras.size(); i++)
	{
		uint32_t cmd_count = renderers.size();
		GraphicsSystem::camera_t *cam = &self->cameras[i];
		uint64_t *renderer_keys = cam->renderer_keys;
		float scale = 1.0f / (cam->zFar - cam->zNear);

		/// Compute new VP

		static const vec3 up(0, 0, 1);
		const mat4 view_matrix = glm::lookAt(cam_views[i].pos, cam_views[i].center, up);
		simd_mul(cam->vp, cam->projection, view_matrix);

		/// Frustum culling

		cam->frustum.init(self->cameras[i].vp);

		for (int j = 0; j < renderers.size(); j++)
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
				static float_int_t max_dist(0x3FFFFFFF);

				vec4 p = cam->frustum.planes[Frustum::Near];
				vec3 c = sphere.center;
				float distance = p.x * c.x + p.y * c.y + p.z * c.z + p.w;
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

		/// Sort renderers

		std::sort(cam->draw_order_indices, cam->draw_order_indices + self->submesh_count,
			[renderer_keys](uint32_t a, uint32_t b) { return renderer_keys[a] < renderer_keys[b]; }
		);

		/// Submit to backend

		//cmd_buffer.draw_renderers(submeshes, self->materials, matrices, RenderPass::Depth,
		//		cam->draw_order_indices, cmd_count);


		if (i != 1) continue;

		Shader::set_builtin("MATRIX_VP", cam->vp);
		Shader::set_builtin("VIEW_POS", cam_views[0].pos);

		GL::Viewport(cam->viewport);
		GL::Scissor (cam->viewport);
		glEnable(GL_CULL_FACE);

		GL::ClearColor(vec4(cam->clear_color, 0.0f));
		glCullFace(GL_BACK);

		glClear(cam->clear_flags);

		vec3 color(1,0,0);
		for (int c = 0; c < cmd_count; c++)
		{
			uint32_t s = cam->draw_order_indices[c];
			submesh_data_t *data = &self->submeshes[s];

			sphere.init(Mesh::meshes.get<3>()[renderers[data->renderer].mesh.id()]);
			sphere.transform(matrices[data->renderer]);

			Debug::sphere(sphere, color);
			color += vec3(0,1,0);
		}

		Debug::frustum(cam->frustum);
	}





	// Setup camera 0
	GraphicsSystem::camera_t *cam = &self->cameras[0];
	Shader::set_builtin("MATRIX_VP", cam->vp);
	Shader::set_builtin("VIEW_POS", cam_views[0].pos);

	GL::Viewport(cam->viewport);
	GL::Scissor (cam->viewport);
	glEnable(GL_CULL_FACE);

	GL::ClearColor(vec4(cam->clear_color, 0.0f));
	glCullFace(GL_BACK);

	glClear(cam->clear_flags);

	for (int j = 0; j < renderers.size(); j++)
	{
		for (int s = renderers[j].first_submesh; s != renderers[j].last_submesh; s++)
		{
			submesh_data_t *data = &self->submeshes[s];

			Shader::set_builtin("MATRIX_M", matrices[data->renderer]);
			//Shader::setBuiltin("MATRIX_N", cmd->model);

			material_t *material = Material::materials.get<0>(data->material);
			material->bind(RenderPass::Forward);

			GL::BindVertexArray(data->submesh.vao);
			glCheck(glDrawElements(data->submesh.mode, data->submesh.count, GL_UNSIGNED_SHORT, (void*)(uint64_t)data->submesh.offset));
		}
	}

	// TODO: should that be called from somwhere else ?
	// see after multi thread sync
	Debug::flush();
}


const system_type_t GraphicsSystem::type = []() {
	system_type_t t{};
	t.name = "GraphicsSystem";
	t.size = sizeof(GraphicsSystem);
	t.on_main_thread = 1;

	t.destroy = [](void *system) { ((GraphicsSystem*)system)->~GraphicsSystem(); };
	t.update = update;
	t.serialize = NULL;
	t.deserialize = NULL;
	return t;
}();
