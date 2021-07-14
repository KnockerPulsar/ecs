#pragma once
#include <vector>
#include "raylib.h"

class Component;
class BallCollision;
class RectCollision;

namespace pong
{
    class BaseCollision : public pong::Component
    {
    public:
        Vector2 *position; // Just in case

        BaseCollision(/* args */) {}
        virtual ~BaseCollision() = 0;
        virtual void Update(std::vector<Component*>* data) = 0;
        // virtual void Update(std::vector<pong::Component *> *data) = 0;

        // Given another base collision, should upcast it into it's proper collision
        // Returns true if a collision occured, false if not.
        // Should have a function for all collider types
        // https://stackoverflow.com/questions/22899363/advice-on-class-structure-in-a-collision-detection-system
        virtual bool CheckCollision(BaseCollision *other) = 0;
        virtual bool CheckCollision(BallCollision *other) = 0;
        virtual bool CheckCollision(RectCollision *other) = 0;

        // Should store logic for collisions
        // Should be Invoked on both collider and colidee, depends on the component implementation
        // TODO: Maybe change things around so that the system is the one checking for collisions, not the component?
        virtual void OnCollision(BaseCollision *other) = 0;
    };
}