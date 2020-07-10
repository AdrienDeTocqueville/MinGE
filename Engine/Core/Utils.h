#pragma once

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

#ifdef DEBUG
#include <iostream>
#endif

#ifdef __linux__
#define DLL_EXPORT	__attribute__((visibility("default")))
#define DLL_IMPORT	__attribute__((visibility("default")))

#define NO_INLINE	__attribute__((noinline))

#ifdef DEBUG
#define ASSERT(x, msg)	if (!(x)) { \
	std::cerr << "Assert in " << __FILE__ << " at line " << __LINE__ << ": '" << # x << "'\n"; \
	__builtin_trap(); std::cin.get(); exit(-1);  }
#else
#efine ASSERT(x, msg)	((void)0)
#endif


#elif _WIN32
#define DLL_EXPORT	__declspec(dllexport)
#define DLL_IMPORT	__declspec(dllimport)

#define NO_INLINE	__declspec(noinline)

#ifdef DEBUG
#include <sstream>
#define ASSERT(x, msg)	if (!(x)) { std::stringstream str; \
	str << "Assert in " << __FILE__ << " at line " << __LINE__ << ": '" << # x << "'\n"; \
	OutputDebugStringA(str.str().c_str()); \
	std::cerr << str.str() << std::endl; \
	DebugBreak(); std::cin.get(); exit(-1);  }
#else
#define ASSERT(x, msg)	((void)0)
#endif

#endif
