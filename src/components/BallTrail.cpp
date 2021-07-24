
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

    BallTrail::BallTrail(int bRef, int numParts, raylib::Texture2D& particleTexture, raylib::Vector2 *ballP, raylib::Color l, raylib::Color r)
    {
        ballPos = ballP;
        numParticles = numParts;
        parts = new Particle *[numParticles];
        for (int i = 0; i < numParticles; i++)
        {
            parts[i] = new Particle;
            parts[i]->owned = true;
            parts[i]->partTexture = &particleTexture;
            inactive.push(parts[i]);
        }
        tag = tags::particle;
        left = l;
        right = r;
        ballRef = bRef;
    }

    BallTrail::~BallTrail()
    {
        for (int i = 0; i < numParticles; i++)
        {
            delete parts[i];
        }
        delete parts;
    }

    void BallTrail::Update()
    {
        Ball *ball = TUtils::GetComponentByType<Ball>(ballRef);
        if (!inactive.empty())
        {
            while (!inactive.empty())
            {
                Particle *part = inactive.front();
                inactive.pop();

                part->position = *ballPos;
                part->lifetime = Utils::GetRand(0.1f, 0.3f);
                part->currTime = 0;
                raylib::Vector2 rand(Utils::GetRand(-100, 100), Utils::GetRand(-100, 100));
                part->vel = rand - ball->velocity * 0.05f;

                if (part->position.x > GetScreenWidth() / 2)
                    part->tint = right;
                else
                    part->tint = left;

                active.push_back(part);
            }
        }
        if (!active.empty())
        {
            for (auto &&part : active)
            {
                part->Update();
                part->CleanUp(this);
            }
        }
    }
} // namespace pong