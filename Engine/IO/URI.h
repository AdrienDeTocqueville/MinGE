#pragma once

#include <unordered_map>
#include <string>
#include <sstream>

#include "Math/glm.h"

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

template<>
inline vec2 parse_val(const std::string &val)
{
	vec2 res;
	sscanf(val.c_str(), "%f,%f", &res.x, &res.y);
	return res;
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
	const bool try_get(const std::string &param, T &output) const
	{
		auto it = params.find(param);
		if (it == params.end())
			return false;

		output = parse_val<T>(it->second);
		return true;
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


	void extract_label(const char *uri, const char *&label, int &len)
	{
		size_t start = path.rfind('/') + 1;
		label = uri + strlen("asset:") + (on_disk ? 2 : 0) + start;

		const char *end = strchr(label, '.');
		if (end == NULL) len = (int)(path.size() - start);
		else len = (int)(end - label);
	}
};
