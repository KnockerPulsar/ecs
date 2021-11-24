#include "RectCollision.h"
#include "BaseCollision.h"
#include "BallCollision.h"
#include "Component.h"
#include <vector>
#include "raylib.h"
#include "../../include/raylib-cpp/raylib-cpp.hpp"

namespace pong
{

    RectCollision::RectCollision(float w, float h) : width(w), height(h) {}

    RectCollision::~RectCollision() {}

    void RectCollision::Update() { DrawDebug(); }

    void RectCollision::Start() { position = &GetEntity()->position; }

    void RectCollision::DrawDebug(raylib::Color col)
    {
        DrawRectangle(position->x, position->y, width, height, col);
        DrawRectangleLines(position->x, position->y, width, height, col);
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

    bool RectCollision::IsNotOnScreen()
    {
        return CheckCollisionRecs(this->GetRect(), raylib::Rectangle(0, 0, GetScreenWidth(), GetScreenHeight()));
    }

} // namespace pong
