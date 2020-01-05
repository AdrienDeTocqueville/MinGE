#include "Utility/Error.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <iostream>
#endif

static std::string getTitle(Error::Type _type)
{
	switch (_type)
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
static int getIcon(Error::Type _type)
{
	switch (_type)
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

void Error::add(Error::Type _type, std::string _description)
{
	if (_type >= Error::MAX)
		_type = Error::MINGE;
	error = true;


#ifdef _WIN32
	MessageBox(nullptr, _description.c_str(), getTitle(_type).c_str(), getIcon(_type));
#else
	std::cout << getTitle(_type).c_str() << ": " << _description << std::endl;
#endif
}

bool Error::check()
{
	if (!error)
		return false;
	error = false;


#ifdef _WIN32
	int r = MessageBox(nullptr, "One or more errors occured.\nDo you really want to continue ?",
					   "MinGE: loading error", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
	return (r == IDNO);
#else
	return true;
#endif
}

Error::Answer Error::ask(Error::Type _type, std::string _question)
{
#ifdef _WIN32
	int r = MessageBox(nullptr, _question.c_str(), getTitle(_type).c_str(), MB_ICONQUESTION | MB_YESNOCANCEL | MB_DEFBUTTON2);
	if (r == IDYES) return Answer::YES;
	if (r == IDNO) return Answer::NO;
	return Answer::CANCEL;
#else
	std::cout << getTitle(_type).c_str() << ": " << _question << " (cancel)" << std::endl;
	return Answer::CANCEL;
#endif
}
