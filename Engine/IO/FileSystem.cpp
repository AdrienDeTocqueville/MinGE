#include "Core/Utils.h"
#include "IO/FileSystem.h"

#include <climits>

#if PLATFORM_LINUX
#include <sys/stat.h>

#elif PLATFORM_WINDOWS
#include <direct.h>
#define MAX_PATH 260

#endif

namespace Path
{

int end_of_path(const std::string &path)
{
	std::size_t last = path.size();
	if (path[path.size() - 1] == '/' || path[path.size() - 1] == '\\')
		last--;

	size_t end1 = path.find_last_of('/', last);
	size_t end2 = path.find_last_of('\\', last);
	size_t end = std::max(end1 + 1, end2 + 1);
	return end == 0 ? std::string::npos : end;
}


std::string dirname(const std::string &path)
{
	return path.substr(0, end_of_path(path));
}

std::string filename(const std::string &path, bool with_extension)
{
	int start = end_of_path(path);
	if (with_extension)
		return path.substr(start);

	std::size_t end = path.find_last_of('.');
	return path.substr(start, end);
}

std::string extension(const std::string &path)
{
	return path.substr(path.find_last_of('.') + 1);
}

std::string absolute(const std::string &path)
{
#if PLATFORM_LINUX
	char result[PATH_MAX];
	char* full = realpath(path.c_str(), result);

#elif PLATFORM_WINDOWS
	char result[MAX_PATH];
	char *full = _fullpath(result, path.c_str(), MAX_PATH);
#endif

	return std::string(full);
}

std::string combine(const std::string &path1, const std::string &path2)
{
	if (path1[path1.size() - 1] == '/' || path1[path1.size() - 1] == '\\')
		return path1 + path2;
	return path1 + '/' + path2;
}

bool create_directory(const std::string &path)
{
#if PLATFORM_LINUX
	const int err = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return err != -1;
	
#elif PLATFORM_WINDOWS
	const int err = _mkdir(path.c_str());
	return err != -1;
#endif
}

}
