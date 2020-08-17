#include <fstream>

#include "tests.h"

#include "Structures/SOA.h"
#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"

void test_entity()
{
	Entity a = Entity::create("a");
	Entity b = Entity::create("b");
	Entity c = Entity::create("c");
	Entity d = Entity::create("d");

	TEST(!Entity::none.is_valid());

	TEST(a.id() == 1);
	TEST(a.is_valid());
	TEST(b.is_valid());
	TEST(c.is_valid());
	TEST(d.is_valid());

	c.destroy();
	TEST(!c.is_valid());

	Entity e = Entity::create("e");
	TEST(e.is_valid());
	TEST(c.id() == e.id());

	TEST(strcmp(e.name(), "e") == 0);

	a.destroy();
	b.destroy();
	d.destroy();
	e.destroy();

	TEST(!a.is_valid());
	TEST(!b.is_valid());
	TEST(!e.is_valid());
	TEST(!e.is_valid());
}

void test_scene()
{
	char name[2] = "a";
	Entity e[6];
	for (int i = 0; i < ARRAY_LEN(e); i++)
	{
		name[0] = 'a' + i;
		e[i] = Entity::create(name);
	}
	e[2].destroy();

	TEST(Entity::get("a") != Entity::none);
	TEST(Entity::get("b") != Entity::none);
	TEST(Entity::get("d") != Entity::none);
	TEST(Entity::get("e") != Entity::none);
	TEST(Entity::get("f") != Entity::none);

	TEST(Entity::get("c") == Entity::none);

	DUMP_SCENE(dump1, "Assets/tests/scene.ge");

	TEST(Entity::get("a") == Entity::none);
	TEST(Entity::get("b") == Entity::none);
	TEST(Entity::get("d") == Entity::none);
	TEST(Entity::get("e") == Entity::none);
	TEST(Entity::get("f") == Entity::none);

	Scene::load("Assets/tests/scene.ge");

	TEST(Entity::get("a") != Entity::none);
	TEST(Entity::get("b") != Entity::none);
	TEST(Entity::get("d") != Entity::none);
	TEST(Entity::get("e") != Entity::none);
	TEST(Entity::get("f") != Entity::none);

	for (int i = 0; i < ARRAY_LEN(e); i++)
		if (i != 2) TEST(e[i].gen() == Entity::get(e[i].id()).gen());

	DUMP_SCENE(dump2, "Assets/tests/scene.ge");

	TEST(dump1 == dump2);
}
