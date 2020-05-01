#include "tests.h"

void benchmark(int iterations)
{
	long time = 0;
	for (int i = 0; i < iterations; i++)
	{
		TransformSystem transforms;
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
	}
	std::cout << time / iterations << "\n";
}

void test_transforms()
{
	auto transforms = new(Engine::alloc_system("TransformSystem", NULL)) TransformSystem();

	/// Create entities
	Entity e1 = Entity::create();
	Entity e2 = Entity::create();
	Entity e3 = Entity::create();
	Entity _e = Entity::create();
	Entity e4 = Entity::create();
	Entity e5 = Entity::create();
	Entity e6 = Entity::create();
	Entity e7 = Entity::create();
	Entity e8 = Entity::create();

	transforms->add(e1, vec3(0, 0, 0));

	transforms->add_child(e1, e2);
	transforms->add_child(e1, e3);
	transforms->add_child(e1, e4, vec3(1, 0, 0));

	transforms->add(_e);

	transforms->add_child(e4, e5, vec3(0, 1, 0));
	transforms->add_child(e4, e6, vec3(1, 0, 0));

	transforms->remove(_e);

	transforms->add_child(e5, e7, vec3(0, 0, 1));
	transforms->add_child(e5, e8, vec3(0, 1, 0));

	transforms->get(e1).set_position(vec3(0.0f));


	Scene::system_ref_t systems[] = {
		Scene::system_ref_t{"world", transforms},
	};
	Scene level(systems, ARRAY_LEN(systems));
	level.save("Assets/", "level", false);
}
