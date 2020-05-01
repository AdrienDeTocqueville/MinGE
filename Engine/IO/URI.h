#pragma once

#include <unordered_map>
#include <string>
#include <sstream>

struct uri_t
{
	bool parse(const char *uri);

	template<typename T>
	T get_or_default(std::string param, T default_value) const
	{
		auto it = params.find(param);
		return it == params.end() ? default_value : parse_val<T>(it->second);
	}

	bool on_disk;
	std::string path;
	std::unordered_map<std::string, std::string> params;

private:
	template<typename T>
	static T parse_val(const std::string &val)
	{
		T res;
		std::istringstream ss(val);
		ss >> res;
		return res;
	}

	template<>
	static bool parse_val(const std::string &val)
	{
		if (val == "true") return true;
		if (val == "false") return false;
		return parse_val<int>(val);
	}
};
