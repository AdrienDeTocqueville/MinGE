#pragma once

#if _WIN32
#define NOMINMAX
#include <Windows.h>
#include <sstream>
#endif

#include <MinGE.h>
#include <iostream>

#include <SDL2/SDL.h>

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

#define LAUNCH(func) do { \
	printf("Testing " #func "...\n"); test_##func(); \
	} while(0)
#define _BENCH(func, iter) do { \
	printf("Mesuring " #func " over %d iterations...\n", iter); \
	benchmark_##func(iter); \
	} while(0)
#ifdef DEBUG
#define BENCH(func, iter) _BENCH(func, 1)
#else
#define BENCH(func, iter) _BENCH(func, iter)
#endif

void benchmark_transforms(int iterations);
void test_transforms();

void test_systems();
void test_entity();

void test_structures();

#define DUMP_SCENE(output, s, path) std::string output; do { \
	s.save(path); s.clear(); \
	std::ifstream file; file.open(path); \
	output = std::string(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>())); \
} while(0)

#ifdef DEBUG
#define REPORT(text, time, ref) std::cout << text << "~" << time << "us\n"
#else
#define REPORT(text, time, ref) std::cout << text << "~" << \
	time << "us (~" << ref << "us for ref)\n"
#endif
