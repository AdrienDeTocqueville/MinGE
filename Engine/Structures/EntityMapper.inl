#pragma once

#include "Structures/EntityMapper.h"
#include "IO/json.hpp"
#include "EntityMapper.h"

template<int N>
entity_mapper_t<N>::entity_mapper_t(const nlohmann::json &dump)
{
	assert(dump.size() == N && "Incorrect data");

	uint32_t max_size = 0;
	for (int n = 0; n < N; n++)
		max_size = std::max(max_size, (uint32_t)dump[n].size());

	size = mem::next_power_of_two(max_size);
	indices = (uint32_t*)calloc(N, size * sizeof(uint32_t));
	for (int n = 0; n < N; n++)
	{
		for (uint32_t i = 0; i < dump[n].size(); i++)
			indices[i + 1 + size * n] = dump[n][i].get<uint32_t>();
	}
}

template<int N>
inline nlohmann::json entity_mapper_t<N>::to_json() const
{
	json arrays = nlohmann::json::array();
	arrays.get_ptr<nlohmann::json::array_t*>()->reserve(N);

	json max_comp = nlohmann::json::array();
	max_comp.get_ptr<nlohmann::json::array_t*>()->reserve(N);

	for (int n = 0; n < N; n++)
	{
		uint32_t m = 0;
		json list = nlohmann::json::array();
		for (uint32_t i = size - 1; i >= 1; i--)
		{
			if (indices[i + size * n] != 0)
			{
				list.get_ptr<nlohmann::json::array_t*>()->reserve(i);

				for (uint32_t j = 1; j <= i; j++)
				{
					list.push_back(indices[j + size * n]);
					m = std::max(indices[j + size * n], m);
				}
				break;
			}
		}
		arrays.push_back(list);
		max_comp.push_back(m);
	}

	json res = json::object();
	res["indices"] = arrays;
	res["max_component"] = max_comp;
	return res;
}