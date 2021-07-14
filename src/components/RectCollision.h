#pragma once
#include "BallCollision.h"

namespace pong
{
    class RectCollision : public pong::BaseCollision
    {
    private:
        float width, height;

    public:
        RectCollision(float w, float h);
        ~RectCollision();
        void Update(std::vector<pong::Component *> *data) override;

        Rectangle GetRect();

        virtual bool CheckCollision(pong::BaseCollision *other);

        // Rect-ball collision
        virtual bool CheckCollision(BallCollision *other);

        // Rect-Rect collision
        virtual bool CheckCollision(RectCollision *other);

        void OnCollision(Component *other);
    };
} // namespace pong
