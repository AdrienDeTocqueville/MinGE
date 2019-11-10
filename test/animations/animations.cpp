#include "animations.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_animations()
{
	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	MeshRef mesh = Mesh::createSphere();

	MaterialRef m = Material::getDefault();
	m->set("albedoMap", Texture::get("Iron/albedo.png"));
	m->set("metallicMap", Texture::get("Iron/metallic.png"));
	m->set("roughnessMap", Texture::get("Iron/roughness.png"));

	Entity *object = Entity::create("Object", true)
		->insert<Graphic>(mesh, std::vector<MaterialRef>{m});

	for (vec3 pos : {vec3(1, 0, 0), vec3(-1, 0, 0), vec3(0, -1, 0), vec3(0, 1, 0)})
		object = Entity::clone(object, pos, vec3(0.0f, 0.0f, Random::next<float>()));

	// Camera
	Entity::create("MainCamera", false, vec3(-2.0f, 0.0f, 2.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(object->find<Transform>(), 0.2f, 1.0f);
}
