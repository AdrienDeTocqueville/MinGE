#pragma once

struct Scene
{
	struct system_ref_t
	{
		const char *name;
		void *instance;
	};

	Scene(Scene &s);
	~Scene();

	bool save(const char *URI);
	void *get_system(const char *name);
	const char *get_system_name(void *system);

	static Scene load(const char *URI);
	static bool save(const char *URI, system_ref_t systems[], int system_count);

private:
	Scene(int count, system_ref_t *refs = NULL);

	int system_count;
	system_ref_t *systems;
};
