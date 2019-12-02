#pragma once

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <cassert>

class LinearAllocator
{
public:
	LinearAllocator(size_t _size):
		size(_size), pool(new uint8_t[size]), current(0)
	{
		assert(current.is_lock_free());
		assert(size % sizeof(uint32_t) == 0);
	}
	~LinearAllocator() { delete[] pool; }

	inline void clear()
	{
		current.store(0, std::memory_order_release);
	}

	inline uint8_t *getStart() const { return pool; }
	inline size_t getSize() const { return current; }

	void *alloc(size_t bytes, uint32_t alignment = 1);

private:
	size_t size;
	uint8_t *pool;
	std::atomic<size_t> current;

	LinearAllocator(const LinearAllocator&) = delete;
	void operator=(const LinearAllocator&) = delete;
};
