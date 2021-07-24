#include "BaseCollision.h"
#include <vector>
#include "raylib.h"
#include "../Entity.h"
#include "Component.h"

namespace pong
{
    class RectCollision;
    class BallCollision;
    class Component;
    class QuadTree;

    BaseCollision::BaseCollision(/* args */) {}
    BaseCollision::~BaseCollision(){};
    void BaseCollision::DrawDebug(raylib::Color col) {}

    // Should store logic for collisions
    // Should be Invoked on both collider and colidee, depends on the component implementation
    // TODO: Maybe change things around so that the system is the one checking for collisions, not the component?
    void BaseCollision::OnCollision(Component *other)
    {
        // Haven't collided before with this obj
        if (collidingWith.find(other->componentID) == collidingWith.end())
        {
            collidingWith.insert({other->componentID, other});
            Entity::GetEntity(entityID)->OnCollisionEnter(this, other);
        }
        else
        {
            TraceLog(LOG_DEBUG, "Collision staying...");
        }
    }

    void BaseCollision::ClearCollision(Component *other)
    {
        // If we're already colliding with this obj
        std::unordered_map<int, pong::Component *>::iterator it;
        if (
            (it = collidingWith.find(other->componentID)) != collidingWith.end())
        {
            collidingWith.erase(it);
            Entity::GetEntity(entityID)->OnCollisionExit(this, other);
        }
    }

}