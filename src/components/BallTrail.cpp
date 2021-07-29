
#include <queue>
#include "Component.h"
#include "Particle.h"
#include "../Utils.h"
#include <algorithm>
#include "BallTrail.h"
#include "Ball.h"
#include "../TUtils.h"

namespace pong
{

    BallTrail::BallTrail(int bRef, int numParts, raylib::Texture2D &particleTexture, raylib::Color l, raylib::Color r)
    {
        numParticles = numParts;
        ballRef = bRef;
        parts = new std::vector<Particle *>(numParticles);

        for (auto &&part : *parts)
        {
            part = new Particle(&particleTexture, this);
            part->scale = 0.01;
            inactive.push(part);
        }

        tag = tags::particle;
        left = l;
        right = r;
    }

    BallTrail::~BallTrail()
    {
        for (auto &&part : *parts)
            delete part;
        delete parts;
    }

    void BallTrail::Start()
    {
        // Cache the ball component on startup
        ball = TUtils::GetComponentFromEntity<Ball>(entityID);
        ballPos = &Entity::GetEntity(ballRef)->position;

        // Initialize the particles' entity since we should be registered to it by now
        for (auto &&part : *parts)
            part->entityID = entityID;
    }

    void BallTrail::Update()
    {
        while (!inactive.empty())
        {
            Particle *part = inactive.front();
            inactive.pop();

            part->position = *ballPos;
            part->lifetime = Utils::GetRand(0.1f, 0.3f);
            raylib::Vector2 rand(Utils::GetRand(-100, 100), Utils::GetRand(-100, 100));
            part->vel = rand - ball->velocity * 0.05f;

            if (part->position.x > GetScreenWidth() / 2)
                part->tint = right;
            else
                part->tint = left;

            active.push_back(part);
        }

        for (auto &&part : active)
        {
            part->Update();
        }
    }
} // namespace pong