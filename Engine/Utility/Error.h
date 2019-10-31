#pragma once

#include <string>

enum ErrorType {WARNING, FILE_NOT_FOUND, OPENGL_ERROR, MINGE_ERROR, USER_ERROR};

class Error
{
	public:
		static void add(ErrorType _type, std::string _description);

		static bool check();

	private:
		static const unsigned maxType = USER_ERROR+1;
		static bool error;
};
