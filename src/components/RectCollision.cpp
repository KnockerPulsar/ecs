#pragma once
#include "BaseCollision.h"
#include "RectCollision.h"
#include "Component.h"
#include <vector>
#include "raylib.h"

namespace pong
{
    RectCollision::RectCollision(float w, float h) : width(w), height(h) {}

    RectCollision::~RectCollision() {}

    void RectCollision::Update(std::vector<pong::Component *> *data) 
    {
    }

    Rectangle RectCollision::GetRect()
    {
        return (Rectangle){position->x, position->y, width, height};
    }

    bool RectCollision::CheckCollision(pong::BaseCollision *other) { return other->CheckCollision(this); }

    // Rect-ball collision
    bool RectCollision::CheckCollision(BallCollision *other)
    {
        Rectangle rect = this->GetRect();
        return CheckCollisionCircleRec(*other->position, other->radius, rect);
    }

    // Rect-Rect collision
    bool RectCollision::CheckCollision(RectCollision *other)
    {
        return CheckCollisionRecs(this->GetRect(), other->GetRect());
    }

    void RectCollision::OnCollision(Component *other)
    {
        TraceLog(LOG_DEBUG, "RECT COLLISION\n");
    }
} // namespace pong
