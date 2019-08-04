#pragma once

#include <initializer_list>

class Random
{
    friend class Engine;

    public:
        static int nextInt(int _min = 0, float _max = 1);
        static float nextFloat(float _min = 0.0, float _max = 1.0);

        static long int getSeed();
        static void setSeed(long int _seed);

        template <typename T>
        static T element(std::initializer_list<T> _elements)
        {
            return *(_elements.begin() + Random::nextInt(0, _elements.size()-1));
        }

    private:
        static void init();

        static long int seed;
};
