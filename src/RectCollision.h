#pragma once
#include "raylib.h"
namespace pong
{
    class BallCollision;
    class RectCollision : public BaseCollision
    {
    private:
        float width, height;
    public:
        RectCollision(Vector2 *pos, float w, float h);
        ~RectCollision();
        void Update(std::vector<Component *> *data);
        void DrawDebug() override;

        Rectangle GetRect();

        virtual bool CheckCollision(Component* other);

        virtual bool CheckCollision(BaseCollision *other);

        // Rect-ball collision
        virtual bool CheckCollision(BallCollision *other);

        // Rect-Rect collision
        virtual bool CheckCollision(RectCollision *other);

        void OnCollision(Component *other) override;
    };
} // namespace pong
