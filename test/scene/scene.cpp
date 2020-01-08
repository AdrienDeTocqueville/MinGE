#include "scene.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_scene()
{
	auto std = Material::getDefault();
	std->define("MAIN_LIGHT");

	Input::setCursorMode(Input::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	SceneRef scene = Scene::create("shadow.gltf");
	scene->spawn();

	// Light
	auto sun = scene->find_entity("Sun");
	if (sun)
	{
		Entity::findByTag("Light")->destroy();
		sun->insert<Graphic>(Mesh::createCube());
	}

	// Camera
	auto cam = scene->find_entity("MainCamera");
	if (!cam)
	{
		cam = Entity::create("MainCamera", false, vec3(0.0f, 2.0f, 2.0f))
			->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f));
	}
	cam->insert<Skybox>();
	cam->insert<CameraScript>(nullptr, 0.2f, 7.0f);
}
