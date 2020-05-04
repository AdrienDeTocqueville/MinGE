#include "tests.h"

#include "Structures/ArrayList.h"

template <typename T, typename S>
void do_test_array_list()
{
	array_list::array_list_t<T, S> arr;

	assert(arr.add(3) == 0);
	assert(arr.add(3) == 3);
	assert(arr.add(3) == 6);

	arr.remove(3, 3);
	assert(arr.add(3) == 3);

	arr.remove(0, 3);
	arr.remove(6, 3);
	arr.remove(3, 3);

	assert(arr.add(3) == 0);
	assert(arr.add(3) == 3);
	assert(arr.add(3) == 6);

	arr.remove(3, 3);
	arr.remove(0, 3);
	assert(arr.add(6) == 0);

	arr.remove(6, 3);
	arr.remove(0, 6);
	assert(arr.add(32) == 0);
}

void test_array_list()
{
	do_test_array_list<uint32_t, array_list::slot32_t>();
	do_test_array_list<uint64_t, array_list::slot64_t>();

	// Should produce a compilation error
	//array_list_t<uint32_t>();
}
