#include <stdlib.h>
#include <string.h>

#include "Structures/EntityMapper.h"
#include "Memory/Memory.h"

uint32_t *entity_mapper_realloc(uint32_t *indices, uint32_t &size, uint32_t new_size, int N)
{
	new_size = mem::next_power_of_two(new_size + 1);
	indices = (uint32_t*)realloc(indices, new_size * N * sizeof(uint32_t));

	for (int n = N; n > 0; n--)
	{
		memcpy(indices + new_size * (n - 1), indices + size * (n - 1), size * sizeof(uint32_t));
		memset(indices + new_size * (n - 1) + size, 0, (new_size - size) * sizeof(uint32_t));
	}

	size = new_size;
	return indices;
}