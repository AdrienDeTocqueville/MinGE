#pragma once

// TODO: rename this file

#define ARRAY_LEN(array) (sizeof(array) / sizeof(*array))

#ifdef __linux__
#define DLL_EXPORT	__attribute__((visibility("default")))
#define DLL_IMPORT	__attribute__((visibility("default")))

#define NO_INLINE	__attribute__((noinline))

#elif _WIN32
#define DLL_EXPORT	__declspec(dllexport)
#define DLL_IMPORT	__declspec(dllimport)

#define NO_INLINE	__declspec(noinline)

#endif
