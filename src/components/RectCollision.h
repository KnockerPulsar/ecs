#pragma once
#include "../../vendor/raylib-cpp/raylib-cpp.hpp"
#include "BaseCollision.h"
namespace pong
{
    class BallCollision;
    
    class RectCollision : public BaseCollision
    {
    public:
        float width, height;

        RectCollision(raylib::Vector2 *pos, float w, float h);
        // RectCollision(float x, float y, float w, float h); 
        ~RectCollision();
        void Update();
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
