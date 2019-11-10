#include "Utility/Memory/LinearAllocator.h"
#include "Utility/Memory/Memory.h"

#include <iostream>

void* LinearAllocator::alloc(size_t bytes, uint32_t alignment)
{
	bytes = align(bytes, alignment);

	uint32_t index = current.fetch_add(bytes, std::memory_order_relaxed);
	uint8_t *ptr = pool + index;

	if (index > size)
	{
		std::cout << "out of memory " << index << " " << size << std::endl;
		return nullptr;
	}

	return ptr;
}
