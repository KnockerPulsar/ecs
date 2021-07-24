#pragma once
#include "../Utils.h"
#include "Component.h"
#include "../../vendor/raylib-cpp/raylib-cpp.hpp"

namespace pong
{

    class Ball : public pong::Component
    {
    private:
        float radius, minSpeed, maxSpeed;
        bool canMove = true;
        raylib::Vector2 *position;
        raylib::Vector2 initialPos;
        raylib::Texture2D *goalParticle;

    public:
        raylib::Vector2 velocity;

        Ball(raylib::Texture2D *goalPart, raylib::Vector2 *position, float radius, float minSpeed, float maxSpeed);
        ~Ball() override;

        void Start() override;

        // angleMin and angleMax are in RADIANS
        void GenerateRandomVelocity(float angleMin, float angleMax, float magMin, float magMax);
        void Move();
        void Update() override;
        void Accelerate();
        void OnCollisionEnter(Component *other) override;
    };

} // namespace pong
