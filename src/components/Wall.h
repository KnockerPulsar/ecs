#pragma once
#include "Component.h"
namespace pong
{
    // Same as with the net, can be replaced by a tag in the entity
    class Wall : public Component
    {
    public:
        Wall() { tag = tags::indep; }
        ~Wall() {}
    };
}