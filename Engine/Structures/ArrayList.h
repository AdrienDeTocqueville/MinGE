#pragma once

#include <cstdint>
#include "Memory/Memory.h"

template<typename T>
struct array_list_t
{
	inline array_list_t();
	~array_list_t() { mem::free_page(data, capacity); }

	void reserve(uint32_t new_cap);
	uint32_t add(uint32_t count);
	void remove(uint32_t index, uint32_t count);
	inline void clear();


	uint32_t next_slot, size, capacity;
	T *data;

	struct slot_t
	{
		uint32_t next, avail;
	};
};


// Implementation
template<typename T>
array_list_t<T>::array_list_t():
	next_slot(0), size(0), capacity(mem::page_size)
{
	// Each element can be used to store the amount of free slots and the next free slot index
	static_assert(sizeof(T) >= sizeof(slot_t), "First element type is too small");

	data = (T*)mem::alloc_page((size_t)capacity * sizeof(T)) - 1;
}

template<typename T>
void array_list_t<T>::reserve(uint32_t new_cap)
{
	void *old_alloc = data + 1;
	void *new_alloc = mem::alloc_page((size_t)new_cap * sizeof(T));

	memcpy(new_alloc, old_alloc, size * sizeof(T));
	data = (T*)new_alloc - 1;

	mem::free_page(old_alloc, capacity);
	capacity = new_cap;
}

template<typename T>
uint32_t array_list_t<T>::add(uint32_t count)
{
	uint32_t slot_id = next_slot;
	uint32_t *prev_slot = &next_slot;
	while (slot_id)
	{
		slot_t *slot = (slot_t*)(data + slot_id);
		if (slot->avail > count) // split slot
		{
			*prev_slot = slot_id + count;
			slot_t *new_slot = (slot_t*)(data + slot_id + count);
			new_slot->next = slot->next;
			new_slot->avail = slot->avail - count;
			return slot_id;
		}
		if (slot->avail == count) // use entire slot
		{
			*prev_slot = slot->next;
			return slot_id;
		}
		slot_id = slot->next;
		prev_slot = &(slot->next);
	}
	if (size + count > capacity)
		reserve(capacity * 2);
	slot_id = size + 1;
	size += count;
	return slot_id;
}

template<typename T>
void array_list_t<T>::remove(uint32_t index, uint32_t count)
{
	slot_t *new_slot, *slot = (slot_t*)(&next_slot);
	while (slot->next != 0 && slot->next < index)
		slot = (slot_t*)(data + slot->next);

	uint32_t slot_id = (T*)slot - data;
	if (slot != (slot_t*)&next_slot && slot_id + slot->avail == index) // merge with prev slot (but never merge with first slot)
	{
		if (slot->next == index + count) // merge with next slot also
		{
			slot_t *merged = (slot_t*)(data + slot->next);
			slot->avail += count + merged->avail;
			slot->next = merged->next;
		}
		else
			slot->avail += count;
		new_slot = slot;
	}
	else if (slot->next == index + count) // merge with next slot
	{
		slot_t *merged = (slot_t*)(data + slot->next);
		slot->next = index;

		new_slot = (slot_t*)(data + index);
		new_slot->next = merged->next;
		new_slot->avail = count + merged->avail;
	}
	else // create new slot
	{
		new_slot = (slot_t*)(data + index);
		new_slot->next = slot->next;
		new_slot->avail = count;
		slot->next = index;
	}

	if (next_slot == 1 && new_slot->avail == size) // everything was removed
		next_slot = size = 0;
	else if ((T*)new_slot + new_slot->avail - data == size + 1) // block is at the end of array
	{
		size -= new_slot->avail;
		slot->next = 0;
	}
}

template<typename T>
void array_list_t<T>::clear()
{
	next_slot = size = 0;
}
