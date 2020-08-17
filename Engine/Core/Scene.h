#pragma once

#include "IO/json_fwd.hpp"

struct Scene
{
	struct system_ref_t
	{
		char *name;
		void *instance;
	};
	enum Overwrite { Yes = 0, No, Ask, Backup };

	static bool load(const char *path);
	static bool save(const char *path, Overwrite mode = Overwrite::Yes);
	static void clear();

	template<typename... R>
	static void set_systems(R... refs)
	{ clear(); reserve(sizeof...(R) / 2); set_system_rec(refs...); }

	static void add_system(char *name, void *instance);
	static void remove_system(const char *name);

	static void *system(const char *name);
	static const char *system_name(void *system);

	static const system_ref_t *systems() { return sys; }
	static int system_count() { return sys_count; }

private:
	Scene() = delete;
	Scene(Scene &&s) = delete;
	Scene(const Scene &s) = delete;
	~Scene();

	static void reserve(int count);

	static void load_assets(nlohmann::json &scene);
	static void save_assets(nlohmann::json &scene);

	template<typename... R>
	static void set_system_rec(const char *name, void *instance, R... refs)
	{
		set_system_rec(name, instance);
		set_system_rec(refs...);
	}
	static void set_system_rec(const char *name, void *instance)
	{
		sys[sys_count].name = strdup(name);
		sys[sys_count].instance = instance;
		sys_count++;
	}

	static int sys_count, capacity;
	static system_ref_t *sys;
};
