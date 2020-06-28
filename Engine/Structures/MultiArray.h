#pragma once

#include <stdint.h>
#include "Core/Utils.h"
#include "Memory/Memory.h"

namespace multi_array
{
	template<int i, typename T>
	struct array_t
	{ T *buf; };

	template<int i, typename... Items>
	struct arrays_t;

	// Base case: empty
	template<int i>
	struct arrays_t<i> {
		constexpr inline uint32_t size() const { return 0; }
		constexpr inline void init(void*, uint32_t) { }
		constexpr inline void move(void*, uint32_t, uint32_t) { }
	};

	// Recursive specialization
	template<int i, typename T, typename... Types>
	struct arrays_t<i, T, Types...> : public array_t<i, T>, public arrays_t<i + 1, Types...>
	{
		constexpr inline uint32_t size() const
		{ return sizeof(T) + arrays_t<i+1, Types...>::size(); }
		constexpr inline void init(void *alloc, uint32_t stride)
		{
			array_t<i, T>::buf = (T*)alloc - 1;
			arrays_t<i+1, Types...>::init(array_t<i, T>::buf + 1 + stride, stride);
		}
		constexpr inline void move(void *alloc, uint32_t size, uint32_t stride)
		{
			memcpy(alloc, array_t<i, T>::buf + 1, size * sizeof(T));
			array_t<i, T>::buf = (T*)alloc - 1;
			arrays_t<i+1, Types...>::move(array_t<i, T>::buf + 1 + stride, size, stride);
		}
	};


	// get the i-th buffer
	template<int i, typename T, typename... Types>
	T *get(const arrays_t<i, T, Types...>& arrays)
	{ return arrays.array_t<i, T>::buf; }
}

template<typename... Types>
struct multi_array_t
{
	inline multi_array_t(uint32_t _capacity = mem::page_size, bool split = true);
	~multi_array_t() { mem::free_page(get<0>() + 1, capacity); }

	void init(uint32_t _size);
	void realloc(uint32_t new_alloc_size);
	uint32_t add();
	inline void remove(uint32_t index);
	inline void clear();

	// get the i-th buffer
	template<int i> constexpr auto get() const
	{ return multi_array::get<i>(data); }

	template<int i> auto get(uint32_t index)
	{
		ASSERT(index != 0 && index <= size, "Invalid index");
		return multi_array::get<i>(data) + index;
	}


	uint32_t next_slot, size, stride, capacity;
	multi_array::arrays_t<0, Types...> data;
};


// Implementation
template<typename... Types>
multi_array_t<Types...>::multi_array_t(uint32_t _capacity, bool split) :
	next_slot(0), size(0), capacity(mem::next_power_of_two(split ? _capacity : _capacity * data.size()))
{
	// The first array is also used to store the next free slot index
	static_assert(sizeof(*get<0>()) >= sizeof(uint32_t), "First element type is too small");

	stride = capacity / data.size();
	void *alloc = mem::alloc_page((size_t)capacity);
	data.init(alloc, stride);
}

template<typename... Types>
void multi_array_t<Types...>::init(uint32_t _size)
{
	size = _size;
	next_slot = 1;

	if (_size > stride)
		realloc(mem::next_power_of_two(_size * data.size()));

	for (uint32_t i(1); i < size; i++)
		*(uint32_t*)(get<0>() + i) = i + 1;
	*(uint32_t*)(get<0>() + size) = 0;
}

template<typename... Types>
void multi_array_t<Types...>::realloc(uint32_t new_alloc_size)
{
	void *old_alloc = get<0>() + 1;
	void *new_alloc = mem::alloc_page((size_t)new_alloc_size);

	stride = new_alloc_size / data.size();
	data.move(new_alloc, size, stride);
	mem::free_page(old_alloc, capacity);

	capacity = new_alloc_size;
}

template<typename... Types>
uint32_t multi_array_t<Types...>::add()
{
	if (const uint32_t ret = next_slot)
	{
		next_slot = *(uint32_t*)(get<0>() + next_slot);
		return ret;
	}
	if (size == stride)
		realloc(capacity * 2);
	return ++size;
}

template<typename... Types>
void multi_array_t<Types...>::remove(uint32_t index)
{
	*(uint32_t*)(get<0>() + index) = next_slot;
	next_slot = index;
}

template<typename... Types>
void multi_array_t<Types...>::clear()
{
	next_slot = size = 0;
}
