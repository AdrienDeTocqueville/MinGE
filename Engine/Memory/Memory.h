#pragma once

namespace mem
{

#ifdef __linux__
#include <sys/mman.h>
#define PAGE_ALLOC(size) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0)
#define PAGE_FREE(address, length) munmap(address, length)

#include <unistd.h>
#define get_page_size() sysconf(_SC_PAGESIZE)

#elif _WIN32
#include <Windows.h>
#define PAGE_ALLOC(size) VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#define PAGE_FREE(address) VirtualFree(address, 0, MEM_RELEASE)

inline unsigned long get_page_size()
{
	SYSTEM_INFO infos;
	GetSystemInfo(&infos);
	return infos.dwPageSize;
}

#endif

const uint32_t page_size = (uint32_t)get_page_size();


inline uint32_t align(uint32_t val, uint32_t alignment)
{
	return ((val + alignment - 1) / alignment ) * alignment;
}

inline uint32_t next_power_of_two(uint32_t x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return ++x;
}

}