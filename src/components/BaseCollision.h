#pragma once
#include <vector>
#include "raylib.h"
#include "Component.h"
#include <unordered_map>
#include "../../vendor/raylib-cpp/raylib-cpp.hpp"

namespace pong
{
    class RectCollision;
    class BallCollision;
    class i;
    class QuadTree;

    class BaseCollision : public Component
    {
    public:
        Vector2 *position; // Just in case
        bool colliding = false;
        QuadTree *currentNode = nullptr;
        std::unordered_map<int, Component *> collidingWith;

        BaseCollision(/* args */);
        virtual ~BaseCollision();
        virtual void Update() = 0;
        virtual void DrawDebug(raylib::Color col = raylib::Color(226, 28, 255, 240));

        // Given another base collision, should upcast it into it's proper collision
        // Returns true if a collision occured, false if not.
        // Should have a function for all collider types
        // https://stackoverflow.com/questions/22899363/advice-on-class-structure-in-a-collision-detection-system
        virtual bool CheckCollision(Component *other) = 0;
        virtual bool CheckCollision(BaseCollision *other) = 0;
        virtual bool CheckCollision(BallCollision *other) = 0;
        virtual bool CheckCollision(RectCollision *other) = 0;

        // Should store logic for collisions
        // Should be Invoked on both collider and colidee, depends on the component implementation
        // TODO: Maybe change things around so that the system is the one checking for collisions, not the component?
        virtual void OnCollision(Component *other);

        virtual void ClearCollision(Component *other);

        virtual bool IsNotOnScreen() = 0;
    };
}