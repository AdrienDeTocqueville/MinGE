#include "physic.h"
#include "../PlayerScript.h"
#include "../CameraScript.h"

void test_physic()
{
	PhysicEngine::get()->setGravity(vec3(0.0f));

	Mesh* cubeMesh = Mesh::createCube();
	Model* boule = new Model("Models/Boule/Boule.obj");

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

	// Other entities
		Entity* player = Entity::create("Player")
			->insert<Graphic>(nullptr)
			->insert<RigidBody>(20.0f)
			->insert<TestPhysic>(sphere)
			->insert<PlayerScript>(10.0f, 2.0f, 30.0f);

		Entity* ground = Entity::clone(plane,
			vec3(0.0f, 0.0f, -10.0f),
			vec3(0.0f),
			vec3(30.0f)
		);

//		plane->clone(vec3(0  , -15, 5 ), vec3(-PI/2, 0	, 0), vec3(30.0f));
//		plane->clone(vec3(0  , 15 , 5 ), vec3(PI/2 , 0	, 0), vec3(30.0f));
//		plane->clone(vec3(15 , 0  , 5 ), vec3(0	, -PI/2, 0), vec3(30.0f));
//		plane->clone(vec3(-15, 0  , 5 ), vec3(0	, PI/2 , 0), vec3(30.0f));

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



	// Camera
	Entity::create("MainCamera")
		->insert<Camera>(70, 0.1f, 1000.0f, vec3(0.67f, 0.92f, 1.0f))
		->insert<Skybox>()

		->insert<CameraScript>(player->find<Transform>(), 0.2f, 7.0f, vec3(0, 0, 0.75f));
}



TestPhysic::TestPhysic(Entity* _prot):
	prot(_prot), dj(nullptr)
{ }

void TestPhysic::start()
{
	Entity* cube = Entity::findByTag("Cube2");
	if (cube != nullptr)
	{
		dj = new DistanceConstraint(find<RigidBody>(), vec3(0, 0, 0.5f),
							   cube->find<RigidBody>(), vec3(0, 0, -0.5f), 3);
//					PhysicEngine::get()->addConstraint(dj);
	}

	shapes[0] = Mesh::createCube();
	shapes[1] = Mesh::createCylinder();
	shapes[2] = Mesh::createCylinder(Material::base, ALLFLAGS, 0.5f, 0.0f);
	shapes[3] = Mesh::createSphere();

	current = -1;
	setShape(0);
}

void TestPhysic::update()
{
	if (Input::getKeyReleased(sf::Keyboard::B))
		PhysicEngine::get()->removeConstraint(dj);
	if (Input::getKeyReleased(sf::Keyboard::N))
		PhysicEngine::get()->addConstraint(dj);

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
