#pragma once

#include <unordered_map>
#include <string>

struct URI
{
	static std::unordered_map<std::string, std::string> parse(char *uri);

	char *uri;
};
