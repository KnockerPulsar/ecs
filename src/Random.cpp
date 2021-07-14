#include <random>
#include <chrono>
namespace Random
{
    // https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
    static float GetRand(float min, float max)
    {
        std::mt19937 generator(time(0));
        std::uniform_int_distribution<int> distribution(min, max);

        return distribution(generator);
    }
};