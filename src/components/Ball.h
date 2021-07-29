#pragma once
#include "../Utils.h"
#include "Component.h"
#include "../../include/raylib-cpp.hpp"

namespace pong
{
    class Net;
    class Paddle;

    class Ball : public pong::Component
    {
    private:
        float radius, minSpeed, maxSpeed, accelRate;
        bool canMove = true;
        raylib::Vector2 *position;
        raylib::Vector2 initialPos;
        raylib::Texture2D *goalParticle;

    public:
        raylib::Vector2 velocity;

        Ball(raylib::Texture2D *goalPart, float radius, float minSpeed, float maxSpeed, float accelRate = 0.001);
        ~Ball() override;

        void Start() override;
        void Reset();

        // angleMin and angleMax are in RADIANS
        void GenerateRandomVelocity(float angleMin, float angleMax, float magMin, float magMax);
        void Move();
        void Update() override;
        void Accelerate();
        void OnCollisionEnter(Component *other) override;
        void PaddleCollision(Paddle* colliderPaddle);
        void NetCollision(Net* collNet);
    };

} // namespace pong
