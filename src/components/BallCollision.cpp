#include "BallCollision.h"
#include <vector>
#include "raylib.h"
#include "Component.h"
#include "../Entity.h"
#include "RectCollision.h"
#include <iostream>
#include "../../include/raylib-cpp.hpp"

namespace pong
{
    // No need to set the tag since it gets set in the BaseCollision ctor
    BallCollision::BallCollision(float radius) : radius(radius) {}
    BallCollision::~BallCollision() {}

    void BallCollision::Update() { DrawDebug(); }

    void BallCollision::DrawDebug(raylib::Color col)
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

    bool BallCollision::IsNotOnScreen()
    {
        return !CheckCollisionCircleRec(*position, radius, raylib::Rectangle(0, 0, GetScreenWidth(), GetScreenHeight()));
    }
} // namespace pong
