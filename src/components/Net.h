#pragma once
#include "Component.h"
namespace pong
{
    // You can think of this as a marker
    // You can also replace this with a tag component in the entity
    // Then set the tag to "net" and check on it in collision functions
    class Net : public Component
    {
    public:
        int pID; // Player entity ID

        Net(int pID)
        {
            tag = tags::indep;
            debug_type = "net component";
            this->pID = pID;
        }
        
        ~Net() {}
    };
}