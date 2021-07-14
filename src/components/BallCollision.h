#pragma once
#include "RectCollision.h"

float MAX_FRAMETIME = 0.01f;
float COLL_CHECK_THRESHOLD;

// Responsible for maintaining the ball's collision
// and detecting its collision with other objects
namespace pong
{
    class BallCollision : public pong::BaseCollision
    {
    private:
    public:
        float radius;

        BallCollision(Vector2 *center, float radius);
        ~BallCollision();

        void Update(std::vector<pong::Component *> *data) override;

        virtual bool CheckCollision(BaseCollision *other);

        // Ball-ball collision
        virtual bool CheckCollision(BallCollision *other);

        // Rect-Ball collision
        virtual bool CheckCollision(RectCollision *other);

        void OnCollision(Component *other);
    };
} // namespace pong
