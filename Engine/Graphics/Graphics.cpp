#include "Graphics/Graphics.h"
#include "Graphics/Shaders/Shader.inl"
#include "Graphics/Shaders/Material.inl"
#include "Graphics/RenderEngine.h"
#include "Graphics/Debug.h"

#include "Transform/Transform.h"

#include "IO/Input.h"


extern multi_array_t<submeshes_t, mesh_data_t, const char*, AABB> mesh_manager;
extern std::vector<submesh_t> submeshes;

GraphicsSystem::GraphicsSystem(TransformSystem *world):
	transforms(world), culling_result_size(0)
{
	//RenderEngine::alloc_cmd_buffer(0);
}

GraphicsSystem::~GraphicsSystem()
{
	for (camera_t &c : cameras)
		free(c.culling_result);
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
	cam->culling_result = (uint8_t*)malloc(culling_result_size);

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
	renderers_entities.emplace_back(entity);
	indices_renderers[entity.id()] = i;

	submeshes_t submeshes = mesh_manager.get<0>()[mesh.id()];

	uint32_t first_material = 1; // 1 is standard material
	if (submeshes.last - submeshes.first != 1)
	{
		// todo
		//first_material = materials.size();
		//for (; submeshes.first < submeshes.last; submeshes.first++)
		//	materials.push_back(default);
	}

	renderers.emplace_back(renderer_t {mesh, submeshes, first_material});

	return i;
}

static void update(void *system)
{
	/*
	// cmd queue construction is single threaded (one thread by camera), see old implementation to multithread it

//  ^   // foreach camera
//  |   // 	// Culling
//  |   // 	foreach renderer
//  MT  // 		if visible
//  |   // 			add to queue
//  |   //
//  |   //	// Sorting
//  v   //	sort queue based on distance, material, mesh index
//	//
//  ^   //	// Depth prepass
//  |   // 	foreach cmd in queue
//  |   // 		if has depth prepass
//  |   // 			render
//  ST  //
//  |   //	// Opaque pass
//  |   // 	foreach cmd in queue
//  |   // 		if has opaque pass
//  v   // 			render
	*/


	GraphicsSystem *self = (GraphicsSystem*)system;
	material_t::bound = nullptr;

	// Read transform data

	struct cam_view_t
	{ vec3 pos, center; };

	// TODO: cache allocations between iterations
	// TODO: mt
	// TODO: use page allocator
	std::vector<mat4> matrices(self->renderers.size());
	std::vector<cam_view_t> cam_views(self->cameras.size());
	{
		//auto lock = transforms.scoped_read_lock();
		for (int i = 0; i < self->renderers_entities.size(); i++)
		{
			Transform tr = self->transforms->get(self->renderers_entities[i]);
			matrices[i] = tr.world_matrix();
		}
		for (int i = 0; i < self->cameras.size(); i++)
		{
			Transform tr = self->transforms->get(self->cameras[i].entity);
			cam_views[i].pos = tr.position();
			cam_views[i].center = tr.to_world(vec3(1, 0, 0));
		}
	}

	// Frustum culling

	if (self->culling_result_size < self->renderers.size())
	{
		self->culling_result_size = self->renderers.size();

		for (int i = 0; i < self->cameras.size(); i++)
		{
			GraphicsSystem::camera_t *cam = &self->cameras[i];
			free(cam->culling_result);
			cam->culling_result = (uint8_t*)malloc(self->culling_result_size);
		}
	}

	Sphere sphere;
	for (int i = 0; i < self->cameras.size(); i++)
	{
		GraphicsSystem::camera_t *cam = &self->cameras[i];

		// Compute new VP
		static const vec3 up(0, 0, 1);
		const mat4 view_matrix = glm::lookAt(cam_views[i].pos, cam_views[i].center, up);
		simd_mul(cam->vp, cam->projection, view_matrix);

		cam->frustum.init(self->cameras[i].vp);

		for (int j = 0; j < self->renderers.size(); j++)
		{
			sphere.init(mesh_manager.get<3>()[self->renderers[j].mesh.id()]);
			sphere.transform(matrices[j]);

			cam->culling_result[j] = Bounds::collide(cam->frustum, sphere);
		}
	}

	// todo...
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

	for (int j = 0; j < self->renderers.size(); j++)
	{
		//material_t *material = &materials[self->renderers[j].first_material];
		material_t *material = Material::materials.get<0>(2);
		for (int s = self->renderers[j].submeshes.first; s != self->renderers[j].submeshes.last; s++)
		{
			if (!cam->culling_result[j])
				continue;
			Shader::set_builtin("MATRIX_M", matrices[j]);
			//Shader::setBuiltin("MATRIX_N", cmd->model);

			material->bind(RenderPass::Forward);
			material++;

			GL::BindVertexArray(submeshes[s].vao);
			glCheck(glDrawElements(submeshes[s].mode, submeshes[s].count, GL_UNSIGNED_SHORT, (void*)(uint64_t)submeshes[s].offset));
		}
	}

	Debug::frustum(self->cameras[1].frustum);

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
