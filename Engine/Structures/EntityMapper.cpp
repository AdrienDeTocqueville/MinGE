#include <stdlib.h>
#include <string.h>

#include "Structures/EntityMapper.h"
#include "Memory/Memory.h"

uint32_t *indices_realloc(uint32_t *indices, uint32_t &size, uint32_t new_size, int N)
{
	new_size = mem::next_power_of_two(new_size + 1);
	indices = (uint32_t*)realloc(indices, new_size * N * sizeof(uint32_t));
	int i = 0;
	do {
		memset(indices + (size + new_size * i), 0, (new_size - size) * sizeof(uint32_t));
	} while (++i != N);

	size = new_size;
	return indices;
}
