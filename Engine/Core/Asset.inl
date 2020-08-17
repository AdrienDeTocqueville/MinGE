#pragma once

#include "IO/json.hpp"


// For some reason, the templated get method of multi_array doesn't compile
// The macro is equivalent to: assets.get<idx>()
#define GET(assets, idx, type) (assets.data.multi_array::array_t<idx, type>::buf)

namespace Asset
{

static inline bool valid_uri(const char *uri)
{
	return (uri != NULL && *uri == 'a');
}

#define ASSET_GET(T, URI, GEN, arr, i) \
	(Asset::valid_uri(arr.get<URI>()[i]) ? T(i, arr.get<GEN>()[i]) : T::none)

using namespace nlohmann;

template <typename Container, typename AssetType>
inline void save(json &dump, const Container &assets, AssetType (*getter)(uint32_t), void (*callback)(uint32_t i, json &dump) = NULL)
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
		if (callback) callback(i, element_dump);
		data.push_back(element_dump);
	}

	dump["max_id"] = max_id;
	dump["data"].swap(data);
}

#include <tuple>

template <typename AssetType, int URI, int GEN, typename... Types>
inline void load(const json &dump, multi_array_t<Types...> &assets, void (*loader)(const json &dump) = NULL)
{
	uint32_t final_slot = 1;
	uint32_t max_id = dump["max_id"].get<uint32_t>();

	// Clear free list
	assets.init(max_id);
	for (uint32_t i(1); i <= max_id; i++)
		GET(assets, URI, char*)[i] = NULL;

	// Populate
	using FirstType = typename std::tuple_element<0, std::tuple<Types...> >::type;
	auto *slots = GET(assets, 0, FirstType);

	const json &data = dump["data"];
	for (auto it = data.rbegin(); it != data.rend(); ++it)
	{
		UID32 uid = it.value()["uint"].get<uint32_t>();

		if (uid.id() == 1) final_slot = *(uint32_t*)(slots + uid.id());
		else *(uint32_t*)(slots + uid.id() - 1) = *(uint32_t*)(slots + uid.id());

		assets.next_slot = uid.id();
		if (loader) loader(it.value());
		else AssetType::load(it.value()["uri"].get<std::string>().c_str());

		GET(assets, GEN, uint8_t)[uid.id()] = uid.gen();
	}
	assets.next_slot = final_slot;
}

template <int URI, typename Container>
inline void clear(Container &assets, void (*callback)(int))
{
	for (uint32_t i(1); i <= assets.size; i++)
	{
		char *uri = GET(assets, URI, char*)[i];
		if (uri == NULL) continue;

		callback(i);
		free(uri);

	}
	assets.clear();
}

}

#undef GET
