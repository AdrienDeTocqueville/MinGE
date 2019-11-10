#include "materials.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_materials()
{
	Random::setSeed(42);

	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	MeshRef mesh = Mesh::createCube();

	MaterialRef m = Material::getDefault();
	Entity *object = Entity::create("Object", true)
		->insert<Graphic>(nullptr);

	const int NUM_MATS = 8;
	MaterialRef mats[NUM_MATS];
	for (int i(0); i < NUM_MATS; i++)
	{
		mats[i] = m->clone();
		mats[i]->set("diffuse", vec3(Random::next<float>(), Random::next<float>(), Random::next<float>()));
	}

	for (int i(0); i < 41; i++)
	for (int j(0); j < 21; j++)
	for (int k(12); k >= 0; k--)
	{
		auto e = Entity::clone(object, vec3(k, i-20.0f, j-10.0f), vec3(0.0f), vec3(0.5f));

		e->find<Graphic>()->setMesh(mesh, {mats[Random::next<int>(0, NUM_MATS)]});
		break;
	}

	// Camera
	Entity::create("MainCamera", false, vec3(-25.0f, 0.0f, 0.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()
		->insert<CameraScript>(nullptr, 0.2f, 7.0f);
}
