#include "scene.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_scene()
{
	Input::setCursorMode(Input::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	Entity::findByTag("Light")->destroy();

	Scene scene("shadow.gltf");
	scene.instantiate();

	Entity::findByTag("Sun", false)
		->insert<Graphic>(Mesh::createCube());

	// Camera
	auto cam = Entity::findByTag("MainCamera");
	if (!cam)
	{
		cam = Entity::create("MainCamera", false, vec3(0.0f, 2.0f, 2.0f))
			->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f));
	}
	cam->insert<Skybox>();
	cam->insert<CameraScript>(nullptr, 0.2f, 7.0f);
}
