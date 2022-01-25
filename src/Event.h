#pragma once
#include <raylib.h>
#include <functional>
#include <vector>

typedef std::function<void(void)> void_fn;

namespace pong
{
    class Event
    {

    public:
        // How long until the event function invokes
        float delay;
        // The function that will be called after `delay` seconds
        void_fn fn;
        // How many times will the event repeat
        int repetitions;
        // How long between repetitions
        float repititionDelay;

        Event(/* args */) {}
        Event(float initDel, void_fn fn, int repetitions = 1, float repDelay = 1)
            : delay(initDel), fn(fn), repetitions(repetitions), repititionDelay(repDelay)
        {
            // Check if the given repitition amount is -ve
            this->repetitions = repetitions < 0 ? 1 : repetitions;
            TraceLog(LOG_WARNING, "Negative repetitions given to event, setting repetitions to 1.");
        }
        ~Event() {}

        bool Tick(float deltaTime)
        {
            delay -= deltaTime;
            if (delay < 0)
            {
                fn();

                // If no more repetitions, mark event for removal
                if (--repetitions == 0)
                    return true;
                // Otherwise, invoke one more time after a delay
                else
                {
                    delay = repititionDelay;
                    return false;
                }
            }
            return false;
        }
    };
}