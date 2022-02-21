#pragma once
#include "../../include/raylib-cpp/include/raylib-cpp.hpp"
#include "BaseCollision.h"
namespace pong
{
    class BallCollision;
    

    // Responsible for maintaining a rectangle/box collision
    // and checking its intersection with other collision types
    class RectCollision : public BaseCollision
    {
    public:
        float width, height;

        RectCollision( float w, float h);
        RectCollision( Vector2* pos, float w, float h);
        ~RectCollision();

        void Update() override;
        void Start() override;

        void DrawDebug(raylib::Color col = raylib::Color::Green()) override;

        Rectangle GetRect();

        virtual bool CheckCollision(Component* other);

        virtual bool CheckCollision(BaseCollision *other);
        // Rect-ball collision
        virtual bool CheckCollision(BallCollision *other);
        // Rect-Rect collision
        virtual bool CheckCollision(RectCollision *other);
        
        virtual bool IsNotOnScreen() override;
     };
} // namespace pong
