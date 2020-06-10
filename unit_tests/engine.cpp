#include "Profiler/profiler.h"

#include "tests.h"

struct TestSystem
{
	char name;
};

void initializer(void *sys)
{
	static int sys_count = 0;

	auto self = (TestSystem*)sys;
	self->name = 'a' + sys_count++;
}

void updater(void *sys)
{
	auto self = (TestSystem*)sys;

	int time = Random::next(50, 400);
	std::cout << "start  " << self->name << "  (" << time << ")" << std::endl;
	//sf::sleep(sf::milliseconds(time));
	std::cout << "finish " << self->name << std::endl;
}

const system_type_t type = []() {
	system_type_t t{};
	t.name = "TestSystem";
	t.size = sizeof(TestSystem);

	t.destroy = NULL;
	t.update = updater;
	t.save = NULL;
	t.load = NULL;
	return t;
}();

const system_type_t type_sync = []() {
	system_type_t t = type;
	t.name = "TestSync";
	return t;
}();

void test_systems()
{
	Engine::register_system_type(type);
	Engine::register_system_type(type_sync);

	auto a = new(Engine::alloc_system("TestSystem")) TestSystem();
	auto b = new(Engine::alloc_system("TestSystem")) TestSystem();
	auto c = new(Engine::alloc_system("TestSystem")) TestSystem();

	TestSystem *ddeps[] = {a};
	auto d = new(Engine::alloc_system("TestSystem")) TestSystem();

	TestSystem *edeps[] = {a, b};
	auto e = new(Engine::alloc_system("TestSync")) TestSystem();

	TestSystem *fdeps[] = {a, b, c};
	auto f = new(Engine::alloc_system("TestSystem")) TestSystem();

	TestSystem *gdeps[] = {b};
	auto g = new(Engine::alloc_system("TestSystem")) TestSystem();

	while (!Input::window_closed())
	//for (int i = 0; i < 10 && !Input::window_closed(); i++)
	{
		std::cout << "\n\n=================\n";
		Engine::frame();
	}
}
