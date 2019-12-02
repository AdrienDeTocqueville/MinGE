#pragma once

#include <stddef.h>
#include <stdint.h>
#include <vector>

template <size_t block_size>
struct StackAllocator
{
	StackAllocator(): block(0), index(0)
	{ blocks.push_back(new uint8_t[block_size]); }

	~StackAllocator()
	{ for (uint8_t *b: blocks) delete[] b;}

	void *alloc(size_t size)
	{
		if (index + size <= block_size)
		{
			void *data = blocks[block] + index;
			index += size;
			return data;
		}
		if (++block == blocks.size())
			blocks.push_back(new uint8_t[block_size]);
		index = size;
		return blocks[block];
	}

	void clear()
	{
		block = 0;
		index = 0;
	}

	uint32_t block, index;
	std::vector<uint8_t*> blocks;
};
