#pragma once

#include <string>

#include "Utility/stb_sprintf.h"

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

	static void add(Error::Type type, const char *description);
	static bool check();

	static Answer ask(Error::Type type, const char *question);

	template<typename... S>
	static void addf(Error::Type type, S... args)
	{
		static char err[256];
		stbsp_snprintf(err, sizeof(err), args...);
		Error::add(type, err);
	}

	static void add(Error::Type type, const std::string &description)
	{
		Error::add(type, description.c_str());
	}

private:
	static bool error;
};
