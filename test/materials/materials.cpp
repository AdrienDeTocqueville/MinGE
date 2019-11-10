#include "materials.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

// (41 * 21 * 3 cubes)
// QUAD = 10.5ms
// TRIANGLES = same
// no bind = 2.4ms
// tr = 6.8ms
// tr + prgm = 7ms
// tr + prgm + texture = 8ms
// everything + glm sse2 = 7ms (but it doesn't work)
// tr + smid = 4ms
//
// inlined(no tex) = 9.0ms;
// uniform blocks = 7.5ms
//
//
// 3 materials, 41*21*4 spheres
// old renderer :  ~5ms
// new renderer, no sort, 1 thread :  ~5.5ms
// new renderer, sort, 4 thread :  ~5.2ms

void test_materials()
{
	Random::setSeed(42);

	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	//MeshRef model = Mesh::load("WarGreymon/WarGreymon.obj");
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
	for (int k(10); k >= 0; k--)
	{
		auto e = Entity::clone(object, vec3(k, i-20.0f, j-10.0f), vec3(0.0f), vec3(0.5f));

		e->find<Graphic>()->setMesh(mesh, {mats[Random::next<int>(0, NUM_MATS)]});
	}

	// Camera
	Entity::create("MainCamera", false, vec3(-25.0f, 0.0f, 0.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<CameraScript>(nullptr, 0.2f, 7.0f);
}
