#include "Graphics/Graphics.h"
#include "Transform/Transform.h"
#include "Graphics/Shaders/Shader.inl"
#include "Graphics/Shaders/Material.inl"

#include "Utility/Error.h"
#include "IO/Input.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <GL/glew.h>

extern multi_array_t<submeshes_t, mesh_data_t, const char*, AABB> mesh_manager;
extern std::vector<submesh_t> submeshes;

GraphicsSystem::GraphicsSystem(TransformSystem *world):
	transforms(world)
{ }

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
	GraphicsSystem *self = (GraphicsSystem*)system;
	material_t::bound = nullptr;

	// TODO: LinearAllocator is thread safe (useless in this case but maybe not for everyone) (using a raw array is better here)
	// TODO: cache allocations between iterations
	// TODO: mt
	// TODO: use page allocator
	std::vector<mat4> matrices(self->renderers.size());
	vec3 view_pos, view_center;
	{
		//auto lock = transforms.scoped_read_lock();
		for (int i = 0; i < self->renderers_entities.size(); i++)
		{
			Transform tr = self->transforms->get(self->renderers_entities[i]);
			matrices[i] = tr.world_matrix();
		}
		Transform cam_tr = self->transforms->get(self->cameras[0].entity);
		view_pos = cam_tr.position();
		view_center = cam_tr.to_world(vec3(1, 0, 0));
	}

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

	// Supports only one camera for now
	// TODO: same as above
	LinearAllocator render_queue(1024 * 1024);
	for (int i = 0; i < cameras.size(); i++)
	{
		for (int j = 0; j < renderers.size(); j++)
		{
			AABB aabb = mesh_manager.get<3>()[renderers[i].mesh.id()];
			aabb.transform(matrices[i]);

			if (!AABB::collide(cameras[i].frustum, aabb))
				continue;
			todo
		}
	}
	*/

	// Compute new VP
	static const vec3 up(0, 0, 1);
	const mat4 view_matrix = glm::lookAt(view_pos, view_center, up);
	mat4 view_proj;
	simd_mul(view_proj, self->cameras[0].projection, view_matrix);

	// Copy data
	Shader::set_builtin("MATRIX_VP", view_proj);
	Shader::set_builtin("VIEW_POS", view_pos);

	GL::Viewport(self->cameras[0].viewport);
	GL::Scissor (self->cameras[0].viewport);
	glEnable(GL_CULL_FACE);

	GL::ClearColor(vec4(self->cameras[0].clear_color, 0.0f));
	glCullFace(GL_BACK);

	glClear(self->cameras[0].clear_flags);

	for (int j = 0; j < self->renderers.size(); j++)
	{
		//material_t *material = &materials[self->renderers[j].first_material];
		material_t *material = Material::materials.get<0>(1);
		for (int s = self->renderers[j].submeshes.first; s != self->renderers[j].submeshes.last; s++)
		{
			Shader::set_builtin("MATRIX_M", matrices[j]);
			//Shader::setBuiltin("MATRIX_N", cmd->model);

			material->bind(RenderPass::Forward);
			material++;

			GL::BindVertexArray(submeshes[s].vao);
			glCheck(glDrawElements(submeshes[s].mode, submeshes[s].count, GL_UNSIGNED_SHORT, (void*)(uint64_t)submeshes[s].offset));
		}
	}
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

namespace RenderEngine
{
void init()
{
	GL::init();
	Shader::setup_builtins();
	//Debug::init();

	Material mat = Material::create(Shader::import("assets://Shaders/standard.json"));
	mat.set("color", vec3(0.8f));
	mat.set("metallic", 0.0f);
	mat.set("roughness", 0.5f);
}

//TODO
// Debug::destroy(); ?
// destroy meshes/materials ?
}
