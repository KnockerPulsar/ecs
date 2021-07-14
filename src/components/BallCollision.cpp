#pragma once
#include <vector>
#include "raylib.h"
#include "Component.h"
#include "BallCollision.h"

// Responsible for maintaining the ball's collision
// and detecting its collision with other objects
namespace pong
{

    BallCollision::BallCollision(Vector2 *center, float radius) : radius(radius) { this->position = center; }
    BallCollision::~BallCollision() {}

    void BallCollision::Update(std::vector<pong::Component *> *data) { }

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

    void BallCollision::OnCollision(Component *other)
    {
        TraceLog(LOG_DEBUG, "BALL COLLISION\n");
    }
} // namespace pong
