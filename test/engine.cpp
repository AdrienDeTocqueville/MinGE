#include "Profiler/profiler.h"

#include "tests.h"

struct TestSystem
{
	int x;
	char *str;
};

void initializer(void *sys)
{
	static int sys_count = 0;

	auto self = (TestSystem*)sys;
	self->x = sys_count++;
	self->str = strdup("          ");
	sprintf(self->str, "updater %d", self->x);
}

void updater(void *sys)
{
	auto self = (TestSystem*)sys;
	MICROPROFILE_SCOPEI("TestSystem", self->str);

	int time = Random::next(50, 400);
	std::cout << "start  " << (char)('a'+self->x) << "  (" << time << ")" << std::endl;
	sf::sleep(sf::milliseconds(time));
	std::cout << "finish " << (char)('a'+self->x) << std::endl;
}

const system_type_t type = []() {
	system_type_t t{};
	t.name = "TestSystem";
	t.size = sizeof(TestSystem);
	t.on_main_thread = 0;

	t.init = initializer;
	t.destroy = NULL;
	t.update = updater;
	t.serialize = NULL;
	t.deserialize = NULL;
	return t;
}();

void test_systems()
{
	Engine::register_system_type(type);
	auto a = (TestSystem*)Engine::create_system("TestSystem", NULL, 0);
	auto b = (TestSystem*)Engine::create_system("TestSystem", NULL, 0);
	auto c = (TestSystem*)Engine::create_system("TestSystem", NULL, 0);

	TestSystem *ddeps[] = {a};
	auto d = (TestSystem*)Engine::create_system("TestSystem", (const void**)ddeps, 1);

	TestSystem *edeps[] = {a, b};
	auto e = (TestSystem*)Engine::create_system("TestSystem", (const void**)edeps, 2);

	TestSystem *fdeps[] = {a, b, c};
	auto f = (TestSystem*)Engine::create_system("TestSystem", (const void**)fdeps, 3);

	TestSystem *gdeps[] = {b};
	auto g = (TestSystem*)Engine::create_system("TestSystem", (const void**)gdeps, 1);

	while (!Input::window_closed())
	//for (int i = 0; i < 10 && !Input::window_closed(); i++)
	{
		std::cout << "\n\n=================\n";
		Engine::start_frame();

		Engine::end_frame();
		Input::window()->display();
	}
}
