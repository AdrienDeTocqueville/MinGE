#include <fstream>

#include "tests.h"

void bench_transforms_creation(int iterations)
{
	long time = 0;
	TransformSystem transforms;
	for (int i = 0; i < iterations; i++)
	{
		Entity entities[1000];

		Time::Chrono timer;
		for (int j = 0; j < 1000; j++)
		{
			entities[j] = Entity::create();
			transforms.add(entities[j], vec3(9, 1, 0));

			for (int k = 0; k < 5; k++)
			{
				Entity e = Entity::create();
				transforms.add_child(entities[j], e);

				for (int l = 0; l < 4; l++)
				{
					Entity c = Entity::create();
					transforms.add_child(e, c);
				}
			}

			transforms.get(entities[j]).set_position(vec3(0.0f));
		}
		time += timer.time();
		transforms.clear();
	}
	REPORT("creating " << 1000 * 5 * 4 << " components: ", time / iterations, 6250);
}

void bench_transforms_access(int iterations)
{
	bool res = false;
	const int count = 1000;
	TransformSystem transforms;

	Entity entities[count];
	for (int i = 0; i < count; i++)
	{
		entities[i] = Entity::create();
		transforms.add(entities[i]);
	}

	long time = 0;
	for (int i = 0; i < iterations; i++)
	{
		Time::Chrono timer;

		for (int k = 0; k < count; k++)
		for (int i = 0; i < count; i++)
			res |= transforms.has(entities[i]);

		time += timer.time();
	}
	TEST(res);

	REPORT("testing " << count * count << " existence: ", time / iterations, 970);
}

void test_transforms()
{
	Engine::register_system_type(TransformSystem::type);
	auto transforms = new(Engine::alloc_system("TransformSystem")) TransformSystem();

	/// Create entities
	Entity::entities.clear();
	Entity e1 = Entity::create("e1");
	Entity e2 = Entity::create("e2");
	Entity e3 = Entity::create("e3");
	Entity _e = Entity::create("_e");
	Entity e4 = Entity::create("e4");
	Entity e5 = Entity::create("e5");
	Entity e6 = Entity::create("e6");
	Entity e7 = Entity::create("e7");
	Entity e8 = Entity::create("e8");

	transforms->add(e1, vec3(0, 0, 0));
	TEST(transforms->has(e1));

	transforms->add_child(e1, e2);
	TEST(transforms->has(e2));

	transforms->add_child(e1, e3);
	TEST(transforms->has(e3));

	transforms->add_child(e1, e4, vec3(1, 0, 0));
	TEST(transforms->has(e4));

	transforms->add(_e);
	TEST(transforms->has(_e));
	TEST(transforms->has(e4));

	transforms->add_child(e4, e5, vec3(0, 1, 0));
	TEST(transforms->has(e5));

	transforms->add_child(e4, e6, vec3(1, 0, 0));
	TEST(transforms->has(e6));

	TEST(transforms->has(_e));
	transforms->remove(_e);
	TEST(!transforms->has(_e));

	transforms->add_child(e5, e7, vec3(0, 0, 1));
	transforms->add_child(e5, e8, vec3(0, 1, 0));

	TEST(transforms->has(e1));
	transforms->get(e1).set_position(vec3(1.0f));

	TEST(transforms->get(e4).position() == vec3(1, 0, 0));

	Scene::set_systems("world", transforms);
	DUMP_SCENE(dump1, "Assets/tests/transforms.ge");

	Engine::register_system_type(TransformSystem::type);
	Scene::load("Assets/tests/transforms.ge");

	DUMP_SCENE(dump2, "Assets/tests/transforms.ge");

	TEST(dump1 == dump2);
}
