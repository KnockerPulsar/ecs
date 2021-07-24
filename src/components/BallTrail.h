#pragma once
#include <queue>
#include "Component.h"
#include "../Utils.h"
#include <algorithm>

namespace pong
{
    class Particle; 

    class BallTrail : public Component
    {
    private:
        int numParticles, ballRef;
        raylib::Vector2 *ballPos;
        raylib::Color left, right;
        Particle** parts;


    public:
        std::vector<Particle *> active;
        std::queue<Particle *> inactive;

        BallTrail(int bRef, int numParts,raylib::Texture2D& particleTexture ,raylib::Vector2 *ballP, raylib::Color l, raylib::Color r);

        ~BallTrail();

        void Update() override;
    };
} // namespace pong
