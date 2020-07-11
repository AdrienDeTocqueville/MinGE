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

	template<typename... R>
	Scene(R... refs): system_count(0), capacity(0), systems(NULL)
	{ reserve(sizeof...(R) / 2); set_system_rec(refs...); }

	Scene(): system_count(0), capacity(0), systems(NULL) {}
	Scene(const char *path): Scene() { load(path); }
	Scene(Scene &&s);
	~Scene();

	bool load(const char *path);
	bool save(const char *path, Overwrite mode = Overwrite::Yes) const;

	template<typename... R>
	void set_systems(R... refs)
	{ clear(); reserve(sizeof...(R) / 2); set_system_rec(refs...); }

	void add_system(char *name, void *instance);
	void remove_system(const char *name);
	void clear();

	void *get_system(const char *name) const;
	const char *get_system_name(void *system) const;

	const system_ref_t *get_systems() const { return systems; }
	int get_system_count() const { return system_count; }

private:
	Scene(const Scene &s) = delete;
	void reserve(int count);

	void load_assets(nlohmann::json &scene);
	void save_assets(nlohmann::json &scene) const;

	template<typename... R>
	void set_system_rec(const char *name, void *instance, R... refs)
	{
		set_system_rec(name, instance);
		set_system_rec(refs...);
	}
	void set_system_rec(const char *name, void *instance)
	{
		systems[system_count].name = strdup(name);
		systems[system_count].instance = instance;
		system_count++;
	}

	int system_count, capacity;
	system_ref_t *systems;
};
