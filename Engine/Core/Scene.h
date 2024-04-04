#pragma once

#include "Core/AssetDatabase.h"
#include "Structures/ArrayList.h"

struct Scene: public UID32
{
	struct scene_t : object_data_t
	{
		std::vector<std::string> system_types;
		std::string serialized_scene;
	};

	Scene() {}
	Scene(UID32 uid): UID32(uid) {}

	void destroy();
	bool is_valid() const { return valid() && scenes[id()].valid(gen()); }

	//uint32_t handle() const { return textures[id()].handle; }

	static Scene create(const char *name);
	static Scene get(uint32_t i) { return UID32(i, scenes[i].gen); }

	static array_list_t<scene_t> scenes;

	static const struct object_type_t type;
	static const struct asset_importer_t importer;
};
