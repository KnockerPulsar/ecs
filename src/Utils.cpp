#include "Utils.h"
#include <random>
namespace pong
{
    // Note: to make this thread-safe, replace the first line with
    //     static std::atomic<std::uint32_t> uid { 0 };  // <<== initialised
    // https://stackoverflow.com/questions/39447118/thread-safe-unique-id-generation-in-c
    int Utils::GetUniqueID()
    {
        static int uid = 0;
        return uid++;
    }

    bool Utils::Between(float less, float value, float more)
    {
        return (less < value) && (value < more);
    }
    bool Utils::BetweenEq(float less, float value, float more)
    {
        return (less <= value) && (value <= more);
    }

    void Utils::XCHG(float &A, float &B)
    {
        float temp = A;
        A = B;
        B = temp;
    }

    float Utils::GetRand(float min, float max)
    {
        static std::mt19937 generator(
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        std::uniform_real_distribution<> distribution(min, max);

        return distribution(generator);
    }

    float Utils::GetRand(float lmin, float lmax, float umin, float umax)
    {
        float l = Utils::GetRand(lmin, lmax);
        float r = Utils::GetRand(umin, umax);
        float choice = Utils::GetRand(0, 1);
        if (choice < 0.5)
            return l;
        else
            return r;
    }

    std::tuple<int, int> Utils::CenterText(std::string str, int posX, int posY, float fontSize)
    {
        return {posX - str.length() * fontSize / 4,
                posY - fontSize / 4};
    }
}
