#pragma once

#include "IO/json.hpp"
#include "Math/glm.h"

struct SerializationContext
{
	void set_version(int v)
	{
		version = v;
	}

	void swap_data(nlohmann::json &dump)
	{
		data.swap(dump);
	}

	template<typename... D>
	void set_dependencies(D... deps)
	{
		free(dependencies);
		dependency_count = sizeof...(D);
		dependencies = (void**)malloc(dependency_count * sizeof(void*));
		save_dependency(deps...);
	}

	const nlohmann::json &operator[](const char *prop) const
	{
		return data[prop];
	}

	void *get_dependency(int i) const
	{
		return dependencies[i];
	}

private:
	SerializationContext(int count = 0): version(0), dependency_count(count),
		dependencies(count ? (void**)malloc(count * sizeof(void*)) : NULL)
	{ }
	~SerializationContext()
	{ free(dependencies); }

	template<typename... D>
	void save_dependency(void *d, D... deps)
	{
		dependencies[dependency_count - sizeof...(D) - 1] = d;
		save_dependency(deps...);
	}

	void save_dependency(void *d)
	{
		dependencies[dependency_count - 1] = d;
	}

	int version;
	nlohmann::json data;

	int dependency_count;
	void **dependencies;

	friend struct Scene;
};

// Serialize
template<length_t L, typename T, qualifier Q>
static inline nlohmann::json to_json(const vec<L, T, Q>& v)
{
	auto res = nlohmann::json::array();
	for (unsigned i(0); i < L; ++i)
		res.push_back(v[i]);
	return res;
}

static inline nlohmann::json to_json(const quat& q)
{
	auto res = nlohmann::json::array();
	for (unsigned i(0); i < 4; ++i)
		res.push_back(q[i]);
	return res;
}

template<length_t R, length_t C, typename T, qualifier Q>
static inline nlohmann::json to_json(const mat<R, C, T, Q>& m)
{
	auto res = nlohmann::json::array();
	for (unsigned i(1); i < R; ++i)
		res.push_back(to_json(m[i]));
	return res;
}

// Deserialize
static inline vec3 to_vec3(const nlohmann::json &x)
{
	return vec3(x[0].get<float>(), x[1].get<float>(), x[2].get<float>());
}

static inline quat to_quat(const nlohmann::json &x)
{
	return quat(x[3].get<float>(), x[0].get<float>(), x[1].get<float>(), x[2].get<float>());
}

static inline vec4 to_vec4(const nlohmann::json &x)
{
	return vec4(x[0].get<float>(), x[1].get<float>(), x[2].get<float>(), x[3].get<float>());
}

// Deserialize or default
template<typename T>
static inline T get_or_default(const nlohmann::json &n, const char *prop, const T &def)
{
	if (!n.contains(prop)) return def;
	return n[prop].get<T>();
}
