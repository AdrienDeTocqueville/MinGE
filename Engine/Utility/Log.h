#pragma once

#include <string>

#include "Utility/stb_sprintf.h"

struct Log
{
	enum Level {
		Info, Warning, Error
	};

	static void add(Level l, const char *description);
	static bool ask(Level l, const char *description);

	template<typename... S>
	static void addf(Level l, S... args)
	{
		static char err[256];
		stbsp_snprintf(err, sizeof(err), args...);
		Log::add(l, err);
	}
};
