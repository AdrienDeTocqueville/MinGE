#pragma once

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

#ifdef DEBUG
#include <iostream>
#endif

#ifdef __linux__
#define PLATFORM_LINUX 1

#define DLL_EXPORT	__attribute__((visibility("default")))
#define DLL_IMPORT	__attribute__((visibility("default")))

#define NO_INLINE	__attribute__((noinline))

#ifdef DEBUG
#define ASSERT(x, msg)	if (!(x)) { \
	std::cerr << "[" << __FILE__ << ":" << __LINE__ << "]: " << msg << ". (" << # x << ")\n"; \
	__builtin_trap(); std::cin.get(); exit(-1);  }
#else
#define ASSERT(x, msg)	((void)0)
#endif


#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS 1

#define DLL_EXPORT	__declspec(dllexport)
#define DLL_IMPORT	__declspec(dllimport)

#define NO_INLINE	__declspec(noinline)

#ifdef _MSC_VER 
#define strcasecmp _stricmp
#endif

#ifdef DEBUG
#include <sstream>
#include "Memory/Memory.h"
#define ASSERT(x, msg)	if (!(x)) { std::stringstream str; \
	str << "[" << __FILE__ << ":" << __LINE__ << "]: " << msg << ". (" << # x << ")\n"; \
	OutputDebugStringA(str.str().c_str()); \
	std::cerr << str.str() << std::endl; \
	DebugBreak(); std::cin.get(); exit(-1);  }
#else
#define ASSERT(x, msg)	((void)0)
#endif

#endif
