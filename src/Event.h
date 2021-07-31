#pragma once
#include <functional>
#include <vector>

namespace pong
{
    class Event
    {

    public:
        float delay;
        std::function<void(void)> fn;

        Event(/* args */) {}
        ~Event() {}

        bool Tick(float deltaTime)
        {
            delay -= deltaTime;
            if (delay < 0)
            {
                fn();
                return true;
            }
            return false;
        }
    };
}