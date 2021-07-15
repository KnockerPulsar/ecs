#pragma once
#include <vector>
#include "raylib.h"
#include "Component.h"
#include "BallCollision.h"
#include <iostream>

// Responsible for maintaining the ball's collision
// and detecting its collision with other objects
namespace pong
{

    BallCollision::BallCollision(Vector2 *center, float radius) : radius(radius)
    {
        this->position = center;
        tag = tags::coll;
    }
    BallCollision::~BallCollision() {}

    void BallCollision::Update(std::vector<pong::Component *> *data) { DrawDebug(); }

    void BallCollision::DrawDebug()
    {
        // Increase debug circle radius by 10 percent so it shows around the circle
        // Might be better to have some drawing order...
        DrawCircle(this->position->x, this->position->y, this->radius * 1.1, GREEN);
    }
    
    bool BallCollision::CheckCollision(Component *other) { return other->CheckCollision(this); }

    bool BallCollision::CheckCollision(BaseCollision *other) { return other->CheckCollision(this); }

    // Ball-ball collision
    bool BallCollision::CheckCollision(BallCollision *other)
    {
        return CheckCollisionCircles(*this->position, this->radius, *other->position, other->radius);
    }

    // Rect-Ball collision
    bool BallCollision::CheckCollision(RectCollision *other)
    {
        return CheckCollisionCircleRec(*this->position, this->radius, other->GetRect());
    }


    // Let's say we collided with one player...
    // We need to get info about that entity
    // Given a pointer to a component, we need a reference to an entity
    // Let's store it in the base component
    void BallCollision::OnCollision(Component *other)
    {
        Entity* collider = Entity::GetEntity(other->entityID);
        Paddle * colliderPaddle = dynamic_cast<Paddle*>(collider->GetComponent(typeid(Paddle)));
         std::string whichPlayer;
        if(colliderPaddle)
        {
            if(colliderPaddle->playerNum == 1) whichPlayer = "left";
            else whichPlayer = "right";
        std::cout << "Collided with the " << whichPlayer << " player\n";
        }

    }
} // namespace pong
