#include "Utility/Random.h"

#include <ctime>

namespace Random
{

static unsigned seed = 0;

void init()
{
	set_seed(time(nullptr));
}

void set_seed(unsigned _seed)
{
	seed = _seed;
	srand(seed);
}

unsigned get_seed()
{
	return seed;
}

}
