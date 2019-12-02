#pragma once

#include "StackAllocator.h"

template <size_t block_size, typename T>
struct BlockAllocator : public StackAllocator<block_size * sizeof(T)>
{
	static const size_t block_bytes = block_size * sizeof(T);
	using Super = StackAllocator<block_bytes>;

	inline T *alloc()
	{
		void *data = Super::alloc(sizeof(T));
		return reinterpret_cast<T*>(data);
	}

	inline size_t count() const
	{
		return	Super::block * block_size +
			Super::index / sizeof(T);
	}

	struct iterator
	{
		uint32_t block, index;
		T *data;
	};

	iterator first() const
	{
		iterator it;
		it.block = 0;
		it.index = 0;
		it.data = (T*)Super::blocks[0];
		return it;
	}

	void next(iterator &it) const
	{
		it.index += sizeof(T);

		// Last block ?
		if (it.block == Super::block)
		{
			// Last element ?
			if (it.index == Super::index)
			{
				it.data = nullptr;
				return;
			}
		}
		// Last element of block ?
		else if (it.index >= block_bytes)
		{
			// Next block
			++it.block;
			it.index = 0;
			it.data = (T*)Super::blocks[it.block];
			return;
		}

		// Next element
		++it.data;
	}
};

