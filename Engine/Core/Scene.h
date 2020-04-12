#pragma once

class Scene
{
public:
	struct system_ref_t
	{
		const char *name;
		void *instance;
	};

	Scene(int _system_count, system_ref_t _systems[]);
	Scene(const char *path, const char *name);
	~Scene();

	Scene(const Scene&) = delete;

	void save(const char *path, const char *name, bool backup_existing);

	void *get_system(const char *name);

private:
	int system_count;
	system_ref_t *systems;
};