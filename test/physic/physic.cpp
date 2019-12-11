#include "physic.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_physic()
{
	//Time::timeScale = 1.0f / 5.0f;

	PhysicEngine::get()->setGravity(vec3(0.0f));

	MeshRef cubeMesh = Mesh::createCube();
	MeshRef boule = Mesh::createSphere();

	// Prototypes
		Entity* sphere = Entity::create("Ball", true)
			->insert<Graphic>(boule)
			->insert<Sphere>(0.5f)
			->insert<RigidBody>(10.0f);

		Entity::create("Cube", true)
			->insert<Graphic>(cubeMesh)
			->insert<Box>()
			->insert<RigidBody>(20.0f);

		Entity::create("Cube2", true)
			->insert<Graphic>(cubeMesh)
			->insert<Box>()
			->insert<RigidBody>(20.0f);

		Entity* plane = Entity::create("Walls", true)
			->insert<Graphic>(Mesh::createQuad())
			->insert<Box>(vec3(0.55f, 0.55f, 0.005f), vec3(0, 0, -0.005f))
			->insert<RigidBody>(0.0f);

	// Playground
		float dim = 8.0f;

		Entity::clone(plane,
			vec3(0.0f, 0.0f, -dim),
			vec3(0.0f),
			vec3(2.0f * dim)
		);


		Entity::clone(plane, vec3(0   , -dim, 0), vec3(-PI/2, 0	, 0), vec3(2.0f * dim));
		Entity::clone(plane, vec3(0   ,  dim, 0), vec3( PI/2, 0	, 0), vec3(2.0f * dim));
		Entity::clone(plane, vec3( dim, 0   , 0), vec3(0	 , -PI/2, 0), vec3(2.0f * dim));
		Entity::clone(plane, vec3(-dim, 0   , 0), vec3(0	 ,  PI/2, 0), vec3(2.0f * dim));

//		new Entity("Plane",
//		{
//			new Graphic(cubeMesh),
//			new Box(),
//			new RigidBody(0.0f),
//			new Transform(vec3(0, 10.5, 1), vec3(0, 0, 0), vec3(30, 10, 1))
//		});
//
//		for( vec3 p: {vec3(-2, 0, 0), vec3(2, 0, 0), vec3(0, -2, 0), vec3(0, 2, 0)} )
//		{
//			Entity* c = cube2->clone(p);
//			DistanceConstraint* d = new DistanceConstraint(c->getComponent<RigidBody>(), vec3(0, 0, 0.5f),
//												 ground->getComponent<RigidBody>(), p*0.033f + vec3(0, 0, 0.33f), 3);
//
//			PhysicEngine::get()->addConstraint(d);
//		}



	// Player
	Entity* player = Entity::create("Player")
		->insert<Graphic>(nullptr)
		->insert<RigidBody>(20.0f)
		->insert<TestPhysic>(sphere)
		->insert<PlayerScript>(30.0f, 15.0f);

	// Camera
	Entity::create("MainCamera")
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()

		->insert<CameraScript>(player->find<Transform>(), 0.2f, 7.0f);
}



TestPhysic::TestPhysic(Entity* _prot):
	prot(_prot)
{ }

void TestPhysic::start()
{
	shapes[0] = Mesh::createCube();
	shapes[1] = Mesh::createCylinder();
	shapes[2] = Mesh::createCylinder(MeshData::Basic, 0.5f, 0.0f);
	shapes[3] = Mesh::createSphere();

	current = -1;
	setShape(0);
}

void TestPhysic::update()
{
	if (Input::getKeyReleased(sf::Keyboard::N) && Entity::findByTag("Ball", false))
	{
		Entity* ball = Random::element(Entity::findAllByTag("Ball", false));

		constraints.push(new DistanceConstraint(
			this->find<RigidBody>(), vec3(0, 0, 0.5f),
			ball->find<RigidBody>(), vec3(0, 0, -0.5f), 3)
		);
		PhysicEngine::get()->addConstraint(constraints.top());
	}
	if (Input::getKeyReleased(sf::Keyboard::B) && !constraints.empty())
	{
		PhysicEngine::get()->removeConstraint(constraints.top());
		constraints.pop();
	}

	if (Input::getKeyReleased(sf::Keyboard::I))
	{
		for (int x(-1) ; x <= 0 ; x++)
		for (int y(0) ; y <= 0 ; y++)
			Entity::clone(prot, vec3(1.5f*x, 1.5f*y, -2));
	}

	auto keys = std::vector<sf::Keyboard::Key>{
		sf::Keyboard::Num1, sf::Keyboard::Num2, sf::Keyboard::Num3, sf::Keyboard::Num4};
	for (unsigned i(0) ; i < keys.size() ; i++)
	{
		if (Input::getKeyReleased(keys[i]))
		{
			setShape(i);
			break;
		}
	}
}
