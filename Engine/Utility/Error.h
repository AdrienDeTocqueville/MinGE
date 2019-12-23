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
		YES, NO, CANCEL
	};

	static void add(Error::Type _type, std::string _description);
	static bool check();

	static Answer ask(Error::Type _type, std::string _question);

private:
	static bool error;
};
