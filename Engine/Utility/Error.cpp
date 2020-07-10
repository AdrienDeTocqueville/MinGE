#include "Utility/Error.h"

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <iostream>
#endif

static const char *get_title(Error::Type type)
{
	switch (type)
	{
	case Error::WARNING:
		return "Warning";
	case Error::FILE_NOT_FOUND:
		return "File not found";
	case Error::OPENGL:
		return "OpenGL error";
	case Error::MINGE:
		return "MinGE error";
	case Error::USER:
		return "User error";

	default:
		return "Unknown error";
	}
}

#ifdef _WIN32
static int get_icon(Error::Type type)
{
	switch (type)
	{
	case Error::WARNING:
		return MB_ICONWARNING;
	case Error::FILE_NOT_FOUND:
		return MB_ICONERROR;
	case Error::OPENGL:
		return MB_ICONERROR;

	default:
		return MB_ICONQUESTION;
	}
}
#endif

bool Error::error = false;

void Error::add(Error::Type type, const char *description)
{
	if (type >= Error::MAX)
		type = Error::MINGE;
	error = true;


#ifdef _WIN32
	MessageBoxA(nullptr, description, get_title(type), get_icon(type));
#else
	std::cout << get_title(type) << ": " << description << std::endl;
#endif
}

bool Error::check()
{
	if (!error)
		return false;
	error = false;


#ifdef _WIN32
	int r = MessageBoxA(nullptr, "One or more errors occured.\nDo you really want to continue ?",
					   "MinGE: loading error", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
	return (r == IDNO);
#else
	return true;
#endif
}

Error::Answer Error::ask(Error::Type type, const char *question)
{
#ifdef _WIN32
	int r = MessageBoxA(nullptr, question, get_title(type), MB_ICONQUESTION | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2);
	if (r == IDTRYAGAIN) return Answer::Retry;
	if (r == IDCONTINUE) return Answer::Ignore;
	return Answer::Cancel;
#else
	std::cout << get_title(type) << ": " << question << " (cancel)" << std::endl;
	return Answer::Cancel;
#endif
}
