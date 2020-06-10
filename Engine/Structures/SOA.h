#pragma once

#include <stdint.h>
#include <assert.h>
#include "Memory/Memory.h"

namespace soa
{
	template<int i, typename T>
	struct array_t
	{ T *buf; };

	template<int i, typename... Items>
	struct arrays_t;

	// Base case: empty
	template<int i>
	struct arrays_t<i> {
		constexpr inline size_t size() const { return 0; }
		constexpr inline void init(void*, uint32_t) { }
		constexpr inline void move(void*, uint32_t, uint32_t) { }
		constexpr inline void move(uint32_t, uint32_t) { }
	};

	// Recursive specialization
	template<int i, typename T, typename... Types>
	struct arrays_t<i, T, Types...> : public array_t<i, T>, public arrays_t<i + 1, Types...>
	{
		constexpr inline size_t size() const
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
		constexpr inline void move(uint32_t dst, uint32_t src)
		{
			memcpy(array_t<i, T>::buf + dst, array_t<i, T>::buf + src, sizeof(T));
			arrays_t<i+1, Types...>::move(dst, src);
		}
	};


	// get the i-th buffer
	template<int i, typename T, typename... Types>
	T *get(const arrays_t<i, T, Types...>& arrays)
	{ return arrays.array_t<i, T>::buf; }
}

// Structure of arrays
// Index 0 is out of bounds
// Index soa_t::size is accessible (if not 0)

template<typename... Types>
struct soa_t
{
	inline soa_t();
	~soa_t() { mem::free_page(get<0>() + 1, capacity); }

	void realloc(uint32_t new_cap);
	uint32_t add();
	inline void remove(uint32_t index);
	inline void clear();

	// get the i-th buffer
	template<int i> constexpr auto get() const
	{ return soa::get<i>(data); }

	uint32_t size, capacity;
	soa::arrays_t<0, Types...> data;
};


// Implementation
template<typename... Types>
soa_t<Types...>::soa_t():
	size(0), capacity(mem::page_size)
{
	void *alloc = mem::alloc_page((size_t)capacity);
	data.init(alloc, capacity / (uint32_t)data.size());
}

template<typename... Types>
void soa_t<Types...>::realloc(uint32_t new_cap)
{
	void *old_alloc = get<0>() + 1;
	void *new_alloc = mem::alloc_page((size_t)new_cap);

	data.move(new_alloc, size, new_cap / (uint32_t)data.size());
	mem::free_page(old_alloc, capacity);

	capacity = new_cap;
}

template<typename... Types>
uint32_t soa_t<Types...>::add()
{
	if (size * data.size() >= capacity)
		realloc(capacity * 2);
	return ++size;
}

template<typename... Types>
void soa_t<Types...>::remove(uint32_t index)
{
	data.move(index, size--);
}

template<typename... Types>
void soa_t<Types...>::clear()
{
	size = 0;
}
