#pragma once

#include "Core/AssetDatabase.h"
#include "Structures/ArrayList.h"

struct Entity: public UID32
{
	struct entity_t : object_data_t
	{
	};

	Entity() {}
	Entity(UID32 uid): UID32(uid) {}

	void destroy();
	bool is_valid() const { return valid() && entities[id()].valid(gen()); }

	const char *name() const { return entities[id()].name; }

	static Entity create(const char *name = NULL);
	static Entity get(uint32_t i) { return UID32(i, entities[i].gen); }
	static Entity get(const char *name);

	static const Entity none;
	static array_list_t<entity_t> entities;

	static const struct object_type_t type;
	//static const struct asset_importer_t importer;
};
