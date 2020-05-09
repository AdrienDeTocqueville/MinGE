#pragma once

#include <stdint.h>
#include "Memory/Memory.h"

namespace array_list
{
struct slot32_t
{
	uint32_t next: 22;
	uint32_t avail: 8;
};
struct slot64_t
{
	uint32_t next;
	uint32_t avail;
};

template<typename T, typename S>
struct array_list_t
{
	inline array_list_t();
	~array_list_t() { mem::free_page(data, capacity); }

	void reserve(uint32_t new_cap);
	uint32_t add(uint32_t count);
	void remove(uint32_t index, uint32_t count);
	inline void clear();

	const uint32_t invalid_id()
	{
		S slot; slot.next = -1;
		return slot.next;
	}

	T &operator[](uint32_t index)
	{ return data[index]; }

	// 64 bit
	union {
		S next_slot;
		struct {
			uint32_t _padding;
			uint32_t capacity;
		};
	};

	uint32_t count, size;
	T *data;
};

template<typename T, typename S>
array_list_t<T, S>::array_list_t():
	capacity(mem::page_size), count(0), size(0)
{
	// Each element can be used to store the amount of free slots and the next free slot index
	static_assert(sizeof(T) >= sizeof(S), "First element type is too small");

	data = (T*)mem::alloc_page((size_t)capacity * sizeof(T));
	next_slot.next = -1;
}

template<typename T, typename S>
void array_list_t<T, S>::reserve(uint32_t new_cap)
{
	void *new_alloc = mem::alloc_page((size_t)new_cap * sizeof(T));

	memcpy(new_alloc, data, size * sizeof(T));
	mem::free_page(data, capacity);

	data = (T*)new_alloc;
	capacity = new_cap;
}

template<typename T, typename S>
uint32_t array_list_t<T, S>::add(uint32_t count)
{
	this->count += count;
	uint32_t slot_id = next_slot.next;
	S *prev_slot = &next_slot;
	while (slot_id != invalid_id())
	{
		S *slot = (S*)(data + slot_id);
		if (slot->avail > count) // split slot
		{
			prev_slot->next = slot_id + count;
			S *new_slot = (S*)(data + slot_id + count);
			new_slot->next = slot->next;
			new_slot->avail = slot->avail - count;
			return slot_id;
		}
		if (slot->avail == count) // use entire slot
		{
			prev_slot->next = slot->next;
			return slot_id;
		}
		slot_id = slot->next;
		prev_slot = slot;
	}
	if (size + count > capacity)
		reserve(capacity * 2);
	slot_id = size;
	size += count;
	return slot_id;
}

template<typename T, typename S>
void array_list_t<T, S>::remove(uint32_t index, uint32_t count)
{
	this->count -= count;
	S *new_slot, *slot = &next_slot;
	while (slot->next != invalid_id() && slot->next < index)
		slot = (S*)(data + slot->next);

	uint32_t slot_id = (uint32_t)((T*)slot - data);
	if (slot != &next_slot && slot_id + slot->avail == index) // merge with prev slot (but never merge with first slot)
	{
		if (slot->next == index + count) // merge with next slot also
		{
			S *merged = (S*)(data + slot->next);
			slot->avail += count + merged->avail;
			slot->next = merged->next;
		}
		else
			slot->avail += count;
		new_slot = slot;
	}
	else if (slot->next == index + count) // merge with next slot
	{
		S *merged = (S*)(data + slot->next);
		slot->next = index;

		new_slot = (S*)(data + index);
		new_slot->next = merged->next;
		new_slot->avail = count + merged->avail;
	}
	else // create new slot
	{
		new_slot = (S*)(data + index);
		new_slot->next = slot->next;
		new_slot->avail = count;
		slot->next = index;
	}

	if (next_slot.next == 0 && new_slot->avail == size) // everything was removed
		clear();
	else if ((T*)new_slot + new_slot->avail - data == size) // block is at the end of array
	{
		size -= new_slot->avail;
		slot->next = -1;
	}
}

template<typename T, typename S>
void array_list_t<T, S>::clear()
{
	next_slot.next = -1;
	size = count = 0;
}
}

template <typename T>
using array_list32_t = array_list::array_list_t<T, array_list::slot32_t>;

template <typename T>
using array_list_t = array_list::array_list_t<T, array_list::slot64_t>;
