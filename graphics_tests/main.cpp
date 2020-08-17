#include <iostream>
#include <MinGE.h>
#include <SDL2/SDL.h>

#include "Render/RenderEngine.h"
#include "Render/Shader/Material.inl"

#include "CameraControl.h"

Mesh cube, rect, sphere;
Texture white, other;

static void dummy_scene(bool serialize = true)
{
	if (serialize) {

	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);
	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	/// Create entities
	Entity mesh_ent = Entity::create("Cube");
	transforms->add(mesh_ent, vec3(2, 0, 0), vec3(0,0,PI*0.25f));
	graphics->add_renderer(mesh_ent, rect);

	mesh_ent = Entity::create("Sphere");
	transforms->add(mesh_ent, vec3(10, 0, 0));
	graphics->add_renderer(mesh_ent, sphere);

	Entity light_ent = Entity::create("Light");
	transforms->add(light_ent, vec3(5, 0, 8));
	graphics->add_point_light(light_ent);

	Entity camera_ent = Entity::create("MainCamera");
	transforms->add(camera_ent, vec3(5, 13, 10));
	auto cam = graphics->add_camera(camera_ent);
	transforms->get(camera_ent).look_at(vec3(5,0,0));
	controller->add(camera_ent);

	camera_ent = Entity::create("Camera2");
	transforms->add(camera_ent, vec3(-6,0,0));
	graphics->add_camera(camera_ent, 70.0f, 2.0f, 10.0f);
	controller->add(camera_ent);

	auto postproc = new(Engine::alloc_system("PostProcessingSystem")) PostProcessingSystem(graphics, cam.depth_texture(), cam.color_texture());


	Scene::set_systems("transforms", transforms, "graphics", graphics, "post-processing", postproc);
	Scene::save("Assets/tests/dummy_scene.ge");

	} else {

	Scene::load("Assets/tests/dummy_scene.ge");

	auto transforms = (TransformSystem*)Scene::system("transforms");
	auto graphics = (GraphicsSystem*)Scene::system("graphics");

	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	controller->add(Entity::get("MainCamera"));
	controller->add(Entity::get("Camera2"));

	//Entity mesh_ent = Entity::get("Sphere");

	}
}

static void gamma_scene(bool serialize = false)
{
	assert(!serialize);

	Scene::load("Assets/Scenes/gamma.ge");

	auto transforms = (TransformSystem*)Scene::system("transforms");
	auto graphics = (GraphicsSystem*)Scene::system("graphics");

	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	controller->add(Entity::get("MainCamera"));
	controller->add(Entity::get("Camera2"));
}

static void pbr_scene(bool serialize = true)
{
	assert(serialize);

	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;

	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);
	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	Material base_material = RenderEngine::default_material;

	/// Create spheres
	glm::mat4 model = glm::mat4(1.0f);
	for (int row = 0; row < nrRows; ++row)
	{
		for (int col = 0; col < nrColumns; ++col)
		{
			float metallic = (float)row / (float)nrRows;
			float roughness = clamp((float)col / (float)nrColumns, 0.05f, 1.0f);

			Material material = Material::copy(base_material);
			material.set("metallic", metallic);
			material.set("roughness", roughness);
			if (col == 2)
			{
				material.define("COLOR_MAP");
				material.set("color_map", other);
			}

			Entity e = Entity::create();
			transforms->add(e, vec3(
				(float)(col - (nrColumns / 2)) * spacing,
				0.0f,
				(float)(row - (nrRows / 2)) * spacing
			));
			graphics->add_renderer(e, sphere, material);
		}
	}

	/// Create lights
	vec3 light_positions[] = {
		vec3(-4.0f, 10.0f,  4.0f),
		vec3( 4.0f, 10.0f,  4.0f),
		vec3(-4.0f, 10.0f, -4.0f),
		vec3( 4.0f, 10.0f, -4.0f),
	};

	for (unsigned int i = 0; i < ARRAY_LEN(light_positions); ++i)
        {
	    Entity light_ent = Entity::create(("Light " + std::to_string(i)).c_str());
	    transforms->add(light_ent, light_positions[i]);
	    graphics->add_point_light(light_ent, vec3(1.0f), 10.0f, 15.0f);
        }


	Entity camera_ent = Entity::create("MainCamera");
	transforms->add(camera_ent, vec3(0, 15, 0));
	transforms->get(camera_ent).look_at(vec3(0,0,0));
	auto cam = graphics->add_camera(camera_ent);
	controller->add(camera_ent);

	auto postproc = new(Engine::alloc_system("PostProcessingSystem")) PostProcessingSystem(graphics, cam.depth_texture(), cam.color_texture());

	Scene::set_systems("transforms", transforms, "graphics", graphics, "post-processing", postproc);
	Scene::save("Assets/Scenes/pbr.ge");
}

static void stress_scene(bool serialize)
{
	assert(serialize);

	Time::set_fps_max(60);

	/// Init systems
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();
	auto graphics = new(Engine::alloc_system("GraphicsSystem")) GraphicsSystem(transforms);
	auto controller = new(Engine::alloc_system("CameraControl")) CameraControl(transforms, graphics);

	RenderEngine::default_material.define("COLOR_MAP");
	RenderEngine::default_material.set("color_map", other);

	static char name[32];
	/// Create entities
	ivec3 count = ivec3(200, 200, 1);
	for (int i(0); i < count.x; i++)
	for (int j(0); j < count.y; j++)
	for (int k(0); k < count.z; k++)
	{
		stbsp_sprintf(name, "Cube %d.%d.%d", i, j, k);
		Entity e = Entity::create(name);
		transforms->add(e, vec3(i + 0.5f * count.x, j, k) - 0.5f * ((vec3)count - 1.0f), vec3(0.0f), vec3(0.75f));
		graphics->add_renderer(e, cube);
	}

	Entity camera_ent = Entity::create("MainCamera");
	transforms->add(camera_ent, vec3(-20.0f, 0.0f, 20.0f));
	auto cam = graphics->add_camera(camera_ent);
	transforms->get(camera_ent).look_at(vec3(500,0,0));
	controller->add(camera_ent);

	auto postproc = new(Engine::alloc_system("PostProcessingSystem")) PostProcessingSystem(graphics, cam.depth_texture(), cam.color_texture());

	/*
	Scene::set_systems("transforms", transforms, "graphics", graphics, "post-processing", postproc);
	Scene::save("Assets/tests/stress_scene.ge");
	*/
}

int main(int, char**)
{
	std::cout << "  -- MinGE --" << std::endl;

	/// Create window
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window = Input::create_window_centered("MinGE", vec2(0.66f),
		SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE, 1);

	/// Init engine
	Engine::init(window);

	Engine::register_system_type(TransformSystem::type);
	Engine::register_system_type(GraphicsSystem::type);
	Engine::register_system_type(PostProcessingSystem::type);
	Engine::register_system_type(CameraControl::type);

	Engine::register_asset_type(Mesh::type);
	Engine::register_asset_type(Texture::type);
	Engine::register_asset_type(Material::type);

	bool serialize = true;

	if (serialize)
	{
		// Open assets
		Engine::load();
		other = Texture::load("asset://Textures/0.png?format=srgb8");
		white = Texture::load("asset://Textures/white.png");

		cube = Mesh::load("asset:mesh/cube?x=1&y=1&z=1");
		rect = Mesh::load("asset:mesh/cube?x=2&y=3&z=3");
		sphere = Mesh::load("asset:mesh/sphere?radius=1");
	}

	//dummy_scene(serialize);
	//gamma_scene(serialize);
	pbr_scene(serialize);
	//stress_scene(serialize);

	/// Main loop
	while (!Input::window_closed())
	{
		if (Input::key_pressed(Key::F8))
			MicroProfileToggleDisplayMode();
		if (Input::key_pressed(Key::F7))
			MicroProfileTogglePause();

		//transforms->get(mesh_ent).rotate(vec3(0,0,1), Time::delta_time);

		Engine::frame();
	}

#ifdef SERIALIZE
	other.destroy();
	white.destroy();

	cube.destroy();
	rect.destroy();
	sphere.destroy();
#else
	Scene::clear();
#endif

	Engine::destroy();

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
