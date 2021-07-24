#pragma once
#include <chrono>

namespace pong
{
    class Component;


    class Utils
    {
    public:
        static int GetUniqueID();
        static bool Between(float less, float value, float more);
        static bool BetweenEq(float less, float value, float more);
        static void XCHG(float &A, float &B);

        // https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
        static float GetRand(float min, float max);
        static float GetRand(float lmin, float lmax, float umin, float umax);

    };

}