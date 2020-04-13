#pragma once

namespace mem
{

#ifdef __linux__
#include <sys/mman.h>
inline void *alloc_page(size_t size) { return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0); }
inline void free_page(void *address, size_t size) { munmap(address, size); }

#include <unistd.h>
inline long get_page_size()
{
	return sysconf(_SC_PAGESIZE);
}

#elif _WIN32
#include <Windows.h>
inline void *alloc_page(size_t size) { return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); }
inline void free_page(void *address, size_t size) { VirtualFree(address, 0, MEM_RELEASE); }

inline unsigned long get_page_size()
{
	SYSTEM_INFO infos;
	GetSystemInfo(&infos);
	return infos.dwAllocationGranularity;
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
