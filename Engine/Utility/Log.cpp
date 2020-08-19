#include "Core/Utils.h"
#include "Utility/Log.h"

#ifdef PLATFORM_WINDOWS
#define NOMINMAX
#include <windows.h>

#else

#endif

static const char *get_title(Log::Level l)
{
	switch (l)
	{
	default:
	case Log::Info:
		return "Information";
	case Log::Warning:
		return "Warning";
	case Log::Error:
		return "Error";
	}
}

#ifdef PLATFORM_WINDOWS
static int get_icon(Log::Level l)
{
	switch (l)
	{
	default:
	case Log::Info:
		return MB_ICONINFORMATION;
	case Log::Warning:
		return MB_ICONWARNING;
	case Log::Error:
		return MB_ICONERROR;
	}
}
#endif

void Log::add(Level l, const char *description)
{
#ifdef PLATFORM_WINDOWS
	MessageBoxA(nullptr, description, get_title(l), get_icon(l));
#else
	printf("%s: %s\n", get_title(l), description);
#endif
}

bool Log::ask(Level l, const char *description)
{
#ifdef PLATFORM_WINDOWS
	int flags = get_icon(l) | MB_DEFBUTTON2 | MB_YESNO;
	return MessageBoxA(nullptr, description, get_title(l), flags) == IDYES;
#else
	printf("%s: %s (y/n)\n", get_title(l), description);
	return getchar() == 'y';
#endif
}
