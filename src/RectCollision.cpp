#pragma once
#include "src/RectCollision.h"
#include "src/BaseCollision.h"
#include "src/BallCollision.h"
#include "src/Component.h"
#include <vector>
#include "raylib.h"

namespace pong
{

    RectCollision::RectCollision(Vector2 *pos, float w, float h) : width(w), height(h)
    {
        position = pos;
        tag = tags::coll;
    }

    RectCollision::~RectCollision() {}

    void RectCollision::Update(std::vector<pong::Component *> *data)
    {
        DrawDebug();
    }

    void RectCollision::DrawDebug()
    {
        // Increase debug rect size by 10 percent for visibility
        DrawRectangleV(*this->position, (Vector2){this->width * 1.1, this->height * 1.1}, GREEN);
    }

    Rectangle RectCollision::GetRect()
    {
        return (Rectangle){position->x, position->y, width, height};
    }

    bool RectCollision::CheckCollision(Component *other) { return other->CheckCollision(this); }

    bool RectCollision::CheckCollision(BaseCollision *other) { return other->CheckCollision(this); }

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
