#pragma once
#include <functional>
#include <vector>

typedef std::function<void(void)> void_fn;

namespace pong
{
    class Event
    {

    public:
        float delay;
        void_fn fn;

        Event(/* args */) {}
        Event(float delay, void_fn fn) : delay(delay), fn(fn) {}
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