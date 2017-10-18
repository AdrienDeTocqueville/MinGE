#include "Random.h"

#include <ctime>
#include <cstdlib>

void Random::init()
{
    srand(time(NULL));
}

int Random::nextInt(int _min, float _max)
{
    return _min + (rand() % (int)(_max - _min + 1));
}

float Random::nextFloat(float _min, float _max)
{
    return _min +  (_max-_min) * rand() / RAND_MAX;
}
