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
		uint32_t next, slot_count;
	};
};


// Implementation
template<typename T>
array_list_t<T>::array_list_t():
	next_slot(0), size(0), capacity(mem::page_size)
{
	// Each element can be used to store the amount of free slots and the next free slot index
	static_assert(sizeof(*get<0>()) >= sizeof(slot_t), "First element type is too small");

	data = (T*)mem::alloc_page((size_t)capacity * sizeof(T)) - 1;
}

template<typename T>
void array_list_t<T>::reserve(uint32_t new_cap)
{
	void *old_alloc = data + 1;
	void *new_alloc = mem::alloc_page((size_t)new_cap * sizeof(T));

	memcpy(new_alloc, old_alloc, size * sizeof(T));
	data = (T*)alloc - 1;

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
		if (slot->slot_count > count) // split slot
		{
			*prev_slot = slot_id + count;
			slot_t *new_slot = (slot_t)*(data + slot_id + count);
			new_slot->next = slot->next;
			new_slot->slot_count = slot->slot_count - count;
			return slot_id;
		}
		if (slot->slot_count == count) // use entire slot
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
	if (next_slot > index) // new slot is first
	{
		slot_t *slot = (slot_t*)(data + index);
		if (next_slot == index + count) // merge with next slot
		{
			slot_t *merged = (slot_t*)(data + next_slot);
			slot->next = merged->next;
			slot->count = count + merged->count;
		}
		else // create new slot
		{
			slot->next = next_slot;
			slot->count = count;
		}
		next_slot = index;
	}
	else
	{
		slot_t *slot = (slot_t*)(data + next_slot);
		while (slot->next != 0 && slot->next < index)
			slot_t *slot = (slot_t*)(data + slot->next);

		if ((T*)slot + slot->slot_count == index) // merge with prev slot
			slot->slot_count += count;
		else if (slot->next == index + count) // merge with next slot
		{
			slot_t *merged = (slot_t*)(data + slot->next);
			slot->next = index;

			slot_t *new_slot = (slot_t*)(data + index);
			new_slot->next = merged->next;
			new_slot->count = count + merged->count;
		}
		else // create new slot
		{
			slot_t *new_slot = (slot_t*)(data + index);
			new_slot->next = slot->next;
			new_slot->count = count;
			slot->next = index;
		}

	}
}

template<typename T>
void array_list_t<T>::clear()
{
	next_slot = size = 0;
}
