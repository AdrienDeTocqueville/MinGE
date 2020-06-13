#include <string.h>
#include "IO/URI.h"

static bool str_diff(const char *ref, const char *src)
{
	while (1)
	{
		if (*ref == '\0') return false;
		if (*ref != *src) return true;

		ref++;
		src++;
	}
}

static const char *find_next(const char *str, char chr)
{
	while (true)
	{
		if (*str == '\0' || *str == chr)
			return str;
		str++;
	}
}

bool uri_t::parse(const char *uri)
{
	const char *colon = strchr(uri, ':');
	if (colon == NULL || str_diff("asset", uri))
		return false;
	on_disk = !str_diff("//", colon + 1);
	const char *path_start = colon + (on_disk ? 2 : 1);
	const char *path_end = find_next(colon + 1, '?');
	if (on_disk) path = "Assets";
	path += std::string(path_start, path_end - path_start);
	if (*path_end == '?')
	{
		const char *arg = path_end;
		do {
			const char *arg_end = strchr(arg + 1, '=');
			if (arg_end == NULL) return false;

			const char *val_end = find_next(arg_end + 1, '&');

			std::string param(arg + 1, arg_end - arg - 1);
			std::string value(arg_end + 1, val_end - arg_end - 1);
			params[std::move(param)] = std::move(value);

			arg = val_end;
		}
		while (*arg);
	}
	return true;
}
