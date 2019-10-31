#include "Utility/Error.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <iostream>
#endif

static std::string getTitle(ErrorType _type)
{
	switch (_type)
	{
	case WARNING:
		return "Warning";
	case FILE_NOT_FOUND:
		return "File not found";
	case OPENGL_ERROR:
		return "OpenGL error";

	default:
		return "MinGE error";
	}
}

#ifdef _WIN32
static int getIcon(ErrorType _type)
{
	switch (_type)
	{
	case WARNING:
		return MB_ICONWARNING;
	case FILE_NOT_FOUND:
		return MB_ICONINFORMATION;
	case OPENGL_ERROR:
		return MB_ICONERROR;

	default:
		return MB_ICONERROR;
	}
}
#endif

bool Error::error = false;

void Error::add(ErrorType _type, std::string _description)
{
	if (_type >= maxType)
		_type = MINGE_ERROR;
	error = true;


#ifdef _WIN32
	MessageBox(nullptr, _description.c_str(), getTitle(_type).c_str(), getIcon(_type));
#else
	std::cout << "Error (" << getTitle(_type).c_str() << "): " << _description << std::endl;
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
