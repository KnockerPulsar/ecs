#pragma once
#include "BaseCollision.h"

// Responsible for maintaining the ball's collision
// and detecting its collision with other objects
namespace pong
{
    class RectCollision;
    class Component;

    class BallCollision : public BaseCollision
    {
    private:
    public:
        float radius;

        BallCollision(Vector2 *center, float radius);
        ~BallCollision();

        void Update() override;
        void DrawDebug(raylib::Color col = raylib::Color::Green()) override;

        virtual bool CheckCollision(Component *other) override;

        virtual bool CheckCollision(BaseCollision *other) override;

        // Ball-ball collision
        virtual bool CheckCollision(BallCollision *other) override;

        // Rect-Ball collision
        virtual bool CheckCollision(RectCollision *other) override;

        virtual bool IsNotOnScreen() override;
    };
} // namespace pong
