#include "Utility/Random.h"

#include <ctime>

long int Random::seed = 0;
Random Random::initalizer;

bool Random::nextBool()
{
	return (double)rand() / RAND_MAX < 0.5;
}


Random::Random()
{
	setSeed( static_cast<long int>(time(nullptr)) );
}

void Random::setSeed(long int _seed)
{
	seed = _seed;
	srand(seed);
}

long int Random::getSeed()
{
	return seed;
}
