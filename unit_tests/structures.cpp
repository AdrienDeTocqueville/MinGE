#include "tests.h"

#include "Structures/ArrayList.h"

template <typename T, typename S>
void do_test_array_list()
{
	array_list::array_list_t<T, S> arr;

	TEST(arr.add(3) == 0);
	TEST(arr.add(3) == 3);
	TEST(arr.add(3) == 6);

	arr.remove(3, 3);
	TEST(arr.add(3) == 3);

	TEST(arr.count == 9);
	TEST(arr.size == 9);
	arr.remove(0, 3);
	arr.remove(6, 3);
	TEST(arr.count == 3);
	TEST(arr.size == 6);
	arr.remove(3, 3);
	TEST(arr.count == 0);
	TEST(arr.size == 0);

	TEST(arr.add(3) == 0);
	TEST(arr.add(3) == 3);
	TEST(arr.add(3) == 6);

	arr.remove(3, 3);
	arr.remove(0, 3);
	TEST(arr.add(6) == 0);

	arr.remove(6, 3);
	arr.remove(0, 6);
	TEST(arr.add(32) == 0);
}

void test_array_list()
{
	do_test_array_list<uint32_t, array_list::slot32_t>();
	do_test_array_list<uint64_t, array_list::slot64_t>();

	// Should produce a compilation error
	//array_list_t<uint32_t>();
}
