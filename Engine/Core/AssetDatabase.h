#pragma once

#include "Core/UID.h"

#include <string>
#include <vector>
#include <cstdlib>

#define INIT_ASSET_TYPE(name) { #name }

typedef uint64_t guid_t;

struct object_type_t
{
	const char *name;

	void (*destroy)(UID32);
	struct object_data_t* (*data)(UID32);
	UID32 (*transfer)(UID32, struct DataModel*);

};

struct object_data_t
{
	enum object_flags_t
	{
		flag_alive = 1 << 0,
	};

	char *name;
	uint8_t flags;
	uint8_t gen;
	void create() { flags |= flag_alive; name = nullptr; }
	bool alive() { return flags & flag_alive; }
	bool valid(uint8_t g) { return alive() && gen == g; }
	void destroy()
	{
		free(name);
		flags &= ~flag_alive;
		++gen;
	}
};

struct asset_importer_t
{
	void (*import)(struct import_context_t &ctx);
};

namespace AssetDatabase
{
	void init(int argc, char **argv);
	void destroy();

	void register_importer(const char *extension, const asset_importer_t &importer);
	guid_t import(const std::string &path);

	UID32 get(const std::string &path);
	UID32 get(const std::string &path, const object_type_t &type, int id);

	//guid_t get_guid(const std::string &path);
	//guid_t get_guid(UID32 object);

	UID32 get(const object_type_t &type, guid_t guid, int id);
	bool get_guid_and_id(UID32 object, const object_type_t &type, guid_t *out_guid, int *out_id);

	std::string resource_path(const std::string &res);

	void register_object_type(const object_type_t &object_type);
	const object_type_t *object_type(const char *name);

	struct subasset_t
	{
		const object_type_t *type;
		UID32 object;
		uint32_t id;
	};
}

struct import_context_t
{
	void register_object(const object_type_t &type, UID32 object);

	const char *path;
	std::vector<AssetDatabase::subasset_t> *subassets;
};