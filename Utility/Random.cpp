#include "Random.h"

#include <ctime>
#include <cstdlib>

#include <iostream>

long int Random::seed;

void Random::init()
{
    setSeed( static_cast<long int>(time(NULL)) );
}

int Random::nextInt(int _min, float _max)
{
    return _min + (rand() % (int)(_max - _min + 1));
}

float Random::nextFloat(float _min, float _max)
{
    return _min +  (_max-_min) * rand() / RAND_MAX;
}

long int Random::getSeed()
{
    return seed;
}

void Random::setSeed(long int _seed)
{
    seed = _seed;
    srand(seed);
}
