#include <cstring>
#include <fstream>
#include <unordered_map>
#include <map>

#include "Core/Utils.h"
#include "Core/AssetDatabase.h"

#include "IO/DataModel.h"
#include "IO/FileSystem.h"

#include "Utility/Log.h"

#define INDEX_FILE "obj/index.db"

static uint64_t generate_guid()
{
	return (((uint64_t)rand()) << 32) | (uint64_t)rand();
}

static uint32_t compute_id(std::vector<AssetDatabase::subasset_t> &subassets, const object_type_t &type)
{
	for (int i = subassets.size() - 1; i >= 0; i--)
	{
		if (strcmp(subassets[i].type->name, type.name) == 0)
			return subassets[i].id + 1;
	}
	return 0;
}

void import_context_t::register_object(const object_type_t &type, UID32 object)
{
	subassets->emplace_back(AssetDatabase::subasset_t { &type, object, compute_id(*subassets, type) });
}

namespace AssetDatabase
{
// Case insensitive string hash
struct strcase_hash
{
	size_t operator()(std::string a) const
	{
		// bad but simple, good enough for extensions
		size_t hash = 13;
		for (int i = 0; i < a.size(); i++)
			hash = hash + ((size_t)(a[i] & ~0x60)) * 27;
		return hash;
	}
};
// Case insensitive string compare
struct strcase_equal
{
	bool operator()(const std::string &a, const std::string &b) const
	{
		return strcasecmp(a.c_str(), b.c_str()) == 0;
	}
};

struct serialized_asset_t
{
	std::string path, data;
	std::vector<subasset_t> subassets;

	int find(const object_type_t &type, int id)
	{
		for (int i = id; i < subassets.size(); i++)
		{
			if (strcmp(subassets[i].type->name, type.name) == 0 && subassets[i].id == id)
				return i;
		}
		return -1;
	}
};


static std::vector<object_type_t> object_types;
static std::unordered_map<std::string, asset_importer_t, strcase_hash, strcase_equal> importers;

static std::unordered_map<std::string, guid_t> path_to_guid;
static std::map<guid_t, serialized_asset_t> guid_to_serialized;
static std::string resource_folder;

void init(int argc, char **argv)
{
	resource_folder = Path::combine(Path::absolute(Path::dirname(argv[0])), "res/");

	std::ifstream file(INDEX_FILE);
	if (!file) return; // Project was never opened

	std::string line;
	while (std::getline(file, line))
	{
		char *guid_end;
		guid_t guid = strtoull(line.c_str(), &guid_end, 16);
		size_t path_start = guid_end + 1 - line.c_str();
		size_t path_end = line.find_first_of(':', path_start);
		std::string path = line.substr(path_start, path_end - path_start);
		path_to_guid[path] = guid;

		serialized_asset_t serialized;
		serialized.data = line.substr(path_end + 1, line.size() - path_end - 1);
		serialized.path = path;

		DataModelReader reader(serialized.data);
		while (!reader.empty())
		{
			subasset_t subasset { NULL, UID32(), 0 };
			reader.transfer(&subasset.type, &subasset.object);
			subasset.id = compute_id(serialized.subassets, *subasset.type);

			serialized.subassets.emplace_back(std::move(subasset));
		}
		guid_to_serialized.insert(std::make_pair(guid, std::move(serialized)));
	}
}

void destroy()
{
	Path::create_directory("obj/");
	std::ofstream file(INDEX_FILE, std::ofstream::out | std::ofstream::trunc);
	if (!file) return;

	for (const auto &pair : guid_to_serialized)
		file << std::hex << pair.first << std::dec << ':' << pair.second.path << ':' << pair.second.data << '\n';
}

void register_importer(const char *extension, const asset_importer_t &importer)
{
	importers[std::string(extension)] = importer;
}

guid_t get_guid(const std::string &path)
{
	auto it = path_to_guid.find(path);
	if (it != path_to_guid.end())
		return it->second;

	return 0;
}

guid_t import(const std::string &path)
{
	// TODO: verify file exists

	std::string ext = Path::extension(path);
	auto imp = importers.find(ext);

	if (imp == importers.end())
		return 0;

	// GUID
	guid_t guid = get_guid(path);
	if (guid == 0)
	{
		guid = generate_guid();
		path_to_guid[path] = guid;
	}

	// Erase previously imported data from db
	serialized_asset_t *serialized;
	serialized_asset_t last_serialized;
	auto last_import = guid_to_serialized.find(guid);
	if (last_import != guid_to_serialized.end())
	{
		last_serialized = std::move(last_import->second);
		serialized = &(last_import->second);
	}
	else
	{
		serialized = &guid_to_serialized[guid];
	}

	// Import and serialize the asset
	{
		// TODO: support import errors
		import_context_t ctx { path.c_str(), &serialized->subassets };
		if (imp->second.import != nullptr)
			imp->second.import(ctx);
		else
		{
			// native asset
			//native_asset_importer(ctx);
		}

		DataModelWriter writer;
		for (subasset_t &sub : serialized->subassets)
			writer.transfer(sub.type, sub.object);

		serialized->path = path;
		serialized->data = writer.to_string();
	}

	// Hot reload
	if (last_serialized.subassets.size() != 0)
	{
		// Reuse ids of subassets that already exist
		for (subasset_t &sub : serialized->subassets)
		{
			int index = last_serialized.find(*sub.type, sub.id);
			if (index != -1)
			{
				subasset_t &last_sub = last_serialized.subassets[index];
				std::swap(last_sub.object, sub.object);
			}
		}

		// We can now delete all assets from last_import
		// They either don't exist anymore, or are duplicated references
		for (const subasset_t &last_sub : last_serialized.subassets)
			last_sub.type->destroy(last_sub.object);

		// Deserialize the asset to reload data and update internal refs
		int i = 0;
		DataModelReader reader(serialized->data);
		while (!reader.empty())
		{
			reader.transfer(&serialized->subassets[i].type, &serialized->subassets[i].object);
			i++;
		}
	}

	return guid;
}

UID32 get(const std::string &path)
{
	guid_t guid = get_guid(path);
	if (guid == 0)
		guid = import(path);

	auto it = guid_to_serialized.find(guid);
	if (it != guid_to_serialized.end())
	{
		serialized_asset_t &serialized = it->second;
		if (serialized.subassets.size() != 0)
			return serialized.subassets[0].object;
	}

	return UID32();
}

UID32 get(const std::string &path, const object_type_t &type, int id)
{
	guid_t guid = get_guid(path);
	if (guid == 0)
		guid = import(path);

	auto it = guid_to_serialized.find(guid);
	if (it != guid_to_serialized.end())
	{
		serialized_asset_t &serialized = it->second;
		int index = serialized.find(type, id);
		if (index != -1)
			return serialized.subassets[index].object;
	}

	return UID32();
}

UID32 get(const object_type_t &type, guid_t guid, int id)
{
	auto last_import = guid_to_serialized.find(guid);
	if (last_import != guid_to_serialized.end())
	{
		const char *name = type.name;
		const auto &assets = last_import->second.subassets;
		for (int i = 0; i < assets.size(); i++)
		{
			if (assets[i].id == id && strcmp(assets[i].type->name, name) == 0)
				return assets[i].object;
		}
	}
	return UID32();
}

bool get_guid_and_id(UID32 object, const object_type_t &type, guid_t *out_guid, int *out_id)
{
	const char *name = type.name;
	for (const auto &pair : guid_to_serialized)
	{
		auto &assets = pair.second.subassets;
		for (int i = 0; i < assets.size(); i++)
		{
			if (assets[i].object == object && strcmp(assets[i].type->name, name) == 0)
			{
				*out_guid = pair.first;
				*out_id = assets[i].id;
				return true;
			}
		}
	}
	return false;
}

std::string resource_path(const std::string &res)
{
	return Path::combine(resource_folder, res);
}

void register_object_type(const object_type_t &object_type)
{
	object_types.emplace_back(object_type);
}

const object_type_t *object_type(const char *name)
{
	for (int i = 0; i < object_types.size(); i++)
	{
		if (strcmp(object_types[i].name, name) == 0)
			return &object_types[i];
	}
	return nullptr;
}

}
