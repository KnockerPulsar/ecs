#include "Includes.h"
#include "Event.cpp"
#include "iostream"
class Object
{
public:
    pong::Transform transform;
    BoundingBox collision;
    bool checkCollisions;
    Object(/* args */) : collision{{0, 0, 0}, {0, 0, 0}}
    {
        events = nullptr;
    }
    virtual ~Object() {}
    virtual void Update(std::vector<Object *> &objs) { CallEvents(); };
    virtual void OnCollision(Object *other){};
    std::vector<Event *> *events; //Unused

    void CallEvents()
    {
        if (events == nullptr)
            return;

        for (int i = 0; i < events->size(); i++)
        {
            (*events)[i]->Tick();
        }
    }

    void AddEvent(void (*fn)(), float duration)
    {
        if (events == nullptr)
            events = new std::vector<Event *>();
        Event *newEvent = new Event(events, duration, fn);

        // Check if the event is already inserted (for multiframe collisions)
        if (std::find(events->begin(), events->end(), newEvent) != events->end())
        {
            events->push_back(newEvent);
            std::cout << "Added event";
        }
    }
};