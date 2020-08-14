#pragma once

#include "IO/json.hpp"

namespace Asset
{

using namespace nlohmann;

template <typename Container, typename AssetType>
inline void save(json &dump, const Container &assets, AssetType (*getter)(uint32_t))
{
	uint32_t max_id = 0;
	json data = json::array();
	data.get_ptr<json::array_t*>()->reserve(assets.size);
	for (uint32_t i(1); i <= assets.size; i++)
	{
		auto element = getter(i);
		if (element == AssetType::none)
			continue;

		max_id = i;
		json element_dump = json::object();
		element_dump["uint"] = element.uint();
		element_dump["uri"] = element.uri();
		data.push_back(element_dump);
	}

	dump["max_id"] = max_id;
	dump["data"].swap(data);
}

template <typename AssetType, int URI, int GEN, typename Container, typename T>
inline void load(const json &dump, Container &assets, T *slot)
{
	uint32_t final_slot = 1;
	uint32_t max_id = dump["max_id"].get<uint32_t>();

	// Clear free list
	assets.init(max_id);
	for (uint32_t i(1); i <= max_id; i++)
		//assets.get<URI>(i) = NULL; // doesn't compile for some reason
		*(assets.data.multi_array::array_t<URI, char*>::buf + i) = NULL;

	// Populate
	const json &data = dump["data"];
	for (auto it = data.rbegin(); it != data.rend(); ++it)
	{
		UID32 uid = it.value()["uint"].get<uint32_t>();

		if (uid.id() == 1) final_slot = *(uint32_t*)(slot + uid.id());
		else *(uint32_t*)(slot + uid.id() - 1) = *(uint32_t*)(slot + uid.id());

		assets.next_slot = uid.id();
		AssetType::load(it.value()["uri"].get<std::string>().c_str());

		//assets.get<GEN>()[uid.id()] = uid.gen(); // doesn't compile for some reason
		*(assets.data.multi_array::array_t<GEN, uint8_t>::buf + uid.id()) = uid.gen();
	}
	assets.next_slot = final_slot;
}

}
