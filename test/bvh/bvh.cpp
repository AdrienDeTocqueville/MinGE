#include "bvh.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_bvh()
{
	Mesh* cubeMesh = Mesh::createCube();

	// Prototypes
		Entity* cube = Entity::create("Cube", true)
			->insert<Graphic>(cubeMesh);

		Entity* plane = Entity::create("Walls", true)
			->insert<Graphic>(Mesh::createQuad())
			->insert<Box>(vec3(0.55f, 0.55f, 0.005f), vec3(0, 0, -0.005f));

	// scene setup
		Entity::clone(plane,
			vec3(0.0f, 0.0f, -10.0f),
			vec3(0.0f),
			vec3(30.0f)
		);

		Entity::clone(cube, vec3(5.0f, 0.0f, 0.0f));
		Entity::clone(cube, vec3(7.0f, 0.0f, 0.0f));
		Entity::clone(cube, vec3(6.0f, 0.0f, -2.0f));

		Entity::clone(cube, vec3(-5.0f, 0.0f, 0.0f));
		Entity::clone(cube, vec3(-7.0f, 0.0f, -2.0f));


	// Camera
	auto cam = Entity::create("MainCamera", false, vec3(0, -14, 0))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()

		->insert<CameraScript>(nullptr, 0.2f, 7.0f, vec3(0, 0, 0.75f));

	cam->find<CameraScript>()->lookAt(vec3(0.0f));
}
