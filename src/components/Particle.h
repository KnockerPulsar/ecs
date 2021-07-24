#pragma once
#include "../../vendor/raylib-cpp/raylib-cpp.hpp"
#include "Component.h"
#include <functional>
#include "BallTrail.h"
#include "../Entity.h"
#define DELET delete
#define DeleteThreshold 20

namespace pong
{
    class Particle : public Component
    {

    public:
        raylib::Texture2D *partTexture;
        raylib::Vector2 position;
        raylib::Vector2 vel;
        raylib::Color tint;
        bool owned = false;

        float rotation = 0, lifetime, currTime = 0, delay = 0;

        Particle() { tag = tags::indep; }
        Particle(raylib::Texture2D *texture, float lifeTime)
        {
            partTexture = texture;
            lifetime = lifeTime;
            tag = tags::particle;
        }
        ~Particle()
        {
            std::string str = "Deleted particle with compID: " + std::to_string(componentID);
            TraceLog(LOG_DEBUG, str.c_str());
        }

        void Update() override
        {
            if (!owned && (position.x < -DeleteThreshold ||
                           position.y < -DeleteThreshold ||
                           position.x > GetScreenWidth() + DeleteThreshold ||
                           position.y > GetScreenHeight() + DeleteThreshold))
            {
                CleanUp(nullptr);
                return;
            }

            float frameTime = std::min(GetFrameTime(), (float)1.0 / 60);
            delay -= frameTime;
            if (delay > 0)
                return;

            partTexture->Draw(position, rotation, 0.01f, tint);

            position += (vel * frameTime);
            currTime += frameTime;
            rotation += frameTime;
        }

        void CleanUp(BallTrail *owner)
        {
            if (currTime > lifetime)
            {
                if (!owned)
                {
                    Entity::GetEntity(entityID)->RemoveComponent(componentID);
                    delete this;
                }
                else
                {
                    auto it = std::find(owner->active.begin(), owner->active.end(), this);
                    if (it != owner->active.end())
                    {
                        owner->active.erase(it);
                        owner->inactive.push(this);
                    }
                }
            }
        }
    };
} // namespace pong
