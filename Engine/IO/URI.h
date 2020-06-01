#pragma once

#include <unordered_map>
#include <string>
#include <sstream>

template<typename T>
inline T parse_val(const std::string &val)
{
	T res;
	std::istringstream ss(val);
	ss >> res;
	return res;
}

template<>
inline bool parse_val(const std::string &val)
{
	if (val == "true") return true;
	if (val == "false") return false;
	return parse_val<int>(val);
}

struct uri_t
{
	bool parse(const char *uri);

	const char *get(const std::string &param) const
	{
		auto it = params.find(param);
		return it == params.end() ? NULL : it->second.data();
	}

	template<typename T>
	T get_or_default(const std::string &param, T default_value) const
	{
		auto it = params.find(param);
		return it == params.end() ? default_value : parse_val<T>(it->second);
	}

	bool on_disk;
	std::string path;
	std::unordered_map<std::string, std::string> params;
};
