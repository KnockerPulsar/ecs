#pragma once
#include <functional>
#include <vector>

namespace pong
{
    class Event
    {
    private:
        std::function<void(void)> fn;
        float delay;

    public:
        static std::vector<Event *> events;

        Event(/* args */) {}
        ~Event() {}

        template <typename func, typename... args>
        static void AddEvent(float delay, func&& fun, args... arguments)
        {
            Event *ev = new Event();
            ev->fn = std::bind(fun, arguments...);
            ev->delay = delay;
            events.push_back(ev);
        }

        template <typename func>
        static void AddEvent(float delay, func&& fun)
        {
            Event *ev = new Event();
            ev->fn = fun;
            ev->delay = delay;
            events.push_back(ev);
        }

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