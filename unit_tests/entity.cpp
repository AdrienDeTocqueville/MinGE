#include <fstream>

#include "tests.h"

#include "Structures/SOA.h"
#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"

static void test_scene()
{
	std::ifstream file;
	char name[2] = "a";
	Entity e[6];
	for (int i = 0; i < ARRAY_LEN(e); i++)
	{
		name[0] = 'a' + i;
		e[i] = Entity::create(_strdup(name));
	}
	e[2].destroy();

	TEST(Entity::get("c") == Entity::none);

	Scene::save("asset://Assets/tests/scene.ge", NULL, 0);
	file.open("Assets/tests/scene.ge");
	std::string dump1(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	file.close();

	for (int i = 0; i < ARRAY_LEN(e); i++)
		if (i != 2) e[i].destroy();

	TEST(Entity::get("a") == Entity::none);
	TEST(Entity::get("b") == Entity::none);
	TEST(Entity::get("d") == Entity::none);
	TEST(Entity::get("e") == Entity::none);
	TEST(Entity::get("f") == Entity::none);

	Scene s = Scene::load("asset://Assets/tests/scene.ge");

	TEST(Entity::get("a") != Entity::none);
	TEST(Entity::get("b") != Entity::none);
	TEST(Entity::get("d") != Entity::none);
	TEST(Entity::get("e") != Entity::none);
	TEST(Entity::get("f") != Entity::none);

	for (int i = 0; i < ARRAY_LEN(e); i++)
		if (i != 2) TEST(e[i].gen() == Entity::get(e[i].id()).gen());

	s.save("asset://Assets/tests/scene.ge");
	file.open("Assets/tests/scene.ge");
	std::string dump2(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	file.close();

	TEST(dump1 == dump2);
}

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

	test_scene();
}