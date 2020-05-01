#include "tests.h"

#include "Structures/ArrayList.h"

void test_array_list()
{
	int x;
	array_list_t<uint64_t> arr;

	x = arr.add(3);
	assert(x == 1);
	x = arr.add(3);
	assert(x == 4);
	x = arr.add(3);
	assert(x == 7);

	arr.remove(4, 3);
	x = arr.add(3);
	assert(x == 4);

	arr.remove(1, 3);
	arr.remove(7, 3);
	arr.remove(4, 3);

	x = arr.add(3);
	assert(x == 1);
	x = arr.add(3);
	assert(x == 4);
	x = arr.add(3);
	assert(x == 7);

	arr.remove(4, 3);
	arr.remove(1, 3);
	x = arr.add(6);
	assert(x == 1);
	arr.remove(7, 3);
	arr.remove(1, 6);
	x = arr.add(32);
	assert(x == 1);
}
