#pragma once

#include <string>


class Error
{
public:
	enum Type {
		WARNING, FILE_NOT_FOUND, OPENGL, MINGE, USER,
		MAX
	};

	enum Answer {
		None, Retry, Ignore, Cancel
	};

	static void add(Error::Type type, std::string description);
	static void add(Error::Type type, const char *description);
	static bool check();

	static Answer ask(Error::Type type, const char *question);

private:
	static bool error;
};
