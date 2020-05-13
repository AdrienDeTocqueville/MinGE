#pragma once

#if _WIN32
#define NOMINMAX
#include <Windows.h>
#include <sstream>
#endif

#include <MinGE.h>
#include <iostream>

#include <SFML/Window.hpp>

#if _WIN32
#define TEST(x) if (!(x)) { std::stringstream str; \
	str << "TEST Failed in " << __FILE__ << " at line " << __LINE__ << ": '" << # x << "'\n"; \
	OutputDebugStringA(str.str().c_str()); \
	std::cerr << str.str() << std::endl; \
	DebugBreak(); std::cin.get(); exit(-1);  }
#else
#define TEST(x) if (!(x)) { \
	printf("TEST Failed in " __FILE__ " at line %d: '" # x "'\n", __LINE__); \
	std::cin.get(); exit(-1); }
#endif

void benchmark_transforms(int iterations);
void test_transforms();

void test_systems();

void test_structures();
