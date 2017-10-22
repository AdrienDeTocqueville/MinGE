#ifndef RANDOM_H
#define RANDOM_H


class Random
{
    friend class Engine;

    public:
        static int nextInt(int _min = 0, float _max = 1);
        static float nextFloat(float _min = 0.0, float _max = 1.0);

        static long int getSeed();
        static void setSeed(long int _seed);

    private:
        static void init();

        static long int seed;
};

#endif // RANDOM_H
