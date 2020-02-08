#include "bvh.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_bvh()
{
	PhysicEngine::get()->setGravity(vec3(0.0f));
	MeshRef cubeMesh = Mesh::createCube();

	// Prototypes
		Entity* cube = Entity::create("Cube", true);
			cube->insert<Graphic>(cubeMesh);

		Entity* plane = Entity::create("Plane", true);
			plane->insert<Graphic>(Mesh::createQuad());

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


	// Player
	Entity* player = Entity::create("Player");
		player->insert<Graphic>(cubeMesh);
		player->insert<Box>();
		player->insert<RigidBody>(20.0f);
		player->insert<PlayerScript>(30.0f, 15.0f);

	// Camera
	Entity *cam = Entity::create("MainCamera");
		cam->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f));
		cam->insert<Skybox>();
		cam->insert<CameraScript>(player->find<Transform>(), 0.2f, 7.0f);

/*
	// Camera
	auto cam = Entity::create("MainCamera", false, vec3(0, -14, 0))
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()

		->insert<CameraScript>(nullptr, 0.2f, 7.0f, vec3(0, 0, 0.75f));

	cam->find<CameraScript>()->lookAt(vec3(0.0f));
*/
}

void MoveScript::update()
{
	vec3 p = tr->position;

	p[axis] = base + sin(Time::time * PI / 2.0f);
	tr->setPosition(p);
}
