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

void test_materials()
{
	Input::setCursorMode(CursorMode::Capture);
	PhysicEngine::get()->setGravity(vec3(0.0f));

	Mesh* cubeMesh = Mesh::createCube();

	Entity *cube = Entity::create("Cube", true)
		->insert<Graphic>(cubeMesh);

	for (int i(0); i < 41; i++)
	for (int j(0); j < 21; j++)
	for (int k(0); k < 3; k++)
		Entity::clone(cube, vec3(k, i-20.0f, j-10.0f), vec3(0.0f), vec3(0.5f));

	// Camera
	Entity::create("MainCamera", false, vec3(-25.0f, 0.0f, 0.0f))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<CameraScript>(nullptr, 0.2f, 7.0f);
}
