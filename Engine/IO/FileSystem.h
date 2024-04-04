#pragma once

#include <string>

namespace Path
{
	std::string dirname(const std::string &path);
	std::string filename(const std::string &path, bool with_extension = true);
	std::string extension(const std::string &path);

	std::string absolute(const std::string &path);
	std::string combine(const std::string &path1, const std::string &path2);

	bool create_directory(const std::string &path);
}