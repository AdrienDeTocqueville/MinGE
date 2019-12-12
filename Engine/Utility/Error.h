#pragma once

#include <string>


class Error
{
public:
	enum Type {
		WARNING, FILE_NOT_FOUND, OPENGL, MINGE, USER,
		MAX
	};

	static void add(Error::Type _type, std::string _description);
	static bool check();

private:
	static bool error;
};
