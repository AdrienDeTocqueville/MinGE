#include "tests.h"

#include "Structures/SOA.h"
#include "Structures/ArrayList.h"
#include "Structures/MultiArray.h"


static void test_multi_array()
{
	multi_array_t<uint64_t> arr;

	TEST(arr.add() == 1);
	TEST(arr.add() == 2);
	TEST(arr.add() == 3);
	TEST(arr.add() == 4);

	TEST(arr.size == 4);

	arr.remove(2);
	TEST(arr.add() == 2);
}



template <typename ArrayType>
static void test_array_list()
{
	ArrayType arr;

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



static void test_soa()
{
	soa_t<uint32_t, uint32_t> arr;

	TEST(arr.add() == 0);
	TEST(arr.add() == 1);
	TEST(arr.add() == 2);

	TEST(arr.size == 3);

	arr.get<0>()[2] = 23;
	arr.get<1>()[2] = 36;

	arr.remove(1);

	TEST(arr.size == 2);
	TEST(arr.get<0>()[1] == 23);
	TEST(arr.get<1>()[2] == 36);
}



void test_structures()
{
	test_multi_array();

	test_array_list<array_list32_t<uint32_t>>();
	test_array_list<array_list_t<uint64_t>>();

	test_soa();

	// Should produce a compilation error
	//array_list_t<uint32_t>();
}
