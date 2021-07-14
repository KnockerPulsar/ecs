#pragma once
#include "Includes.h"
class Event
{
public:
    void (*fn)();   // Function pointer
    float timeLeft; // timeLeft of the event
    std::vector<Event *> *eventsVec;

    Event(std::vector<Event *> *events, float dur, void (*func)()) : eventsVec(events)
    {
        fn = func;
        timeLeft = dur;
    }

    void Tick()
    {
        timeLeft -= GetFrameTime();
        if (timeLeft < 0)
        {
            // Remove the current event from the event queue;
            eventsVec->erase(std::find(eventsVec->begin(), eventsVec->end(), this));
            delete this;
            return;
        }
        fn();
    }
};