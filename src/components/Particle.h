#pragma once
#include "../../include/raylib-cpp/raylib-cpp.hpp"
#include "Component.h"
#include <functional>
#include "BallTrail.h"
#include "../Entity.h"
#include "../TUtils.h"
#include "../System.h"
#define DELET delete
#define DeleteThreshold 20

namespace pong
{

    // Responsible for displaying and "animating" particle effects
    // Particles can be independant and delete themselves when their lifetime is up or when they're out of the screen
    // They can also be owned to some other component/entity that controls their life (for pooling for example)
    class Particle : public Component
    {

    public:
        raylib::Texture2D *partTexture; // Particle texture used to render this particle
        raylib::Vector2 position;       // Current particle position
        raylib::Vector2 vel;            // Current particle velocity
        raylib::Color tint;             // Particle coloration, best used with a white particle
        Component *owner = nullptr;

        float
            rotation = 0,   // Particle rotation in 2D
            lifetime = 0,   // How long the particle will be rendered for
            delay = 0,      // The delay before this particle starts rendering
            rotsPerSec = 1, // How much the particle rotates per frame
            scale = 1;

        Particle() { tag = tags::particle; }

        Particle(raylib::Texture2D *texture, float lifeTime)
        {
            partTexture = texture;
            lifetime = lifeTime;
            tag = tags::particle;
        }

        Particle(raylib::Texture2D *texture, Component *owner)
        {
            partTexture = texture;
            this->owner = owner;
        }

        // Remove yourself from the system
        ~Particle()
        {
            if (!owner)
            {
                System::RemoveComponentIndep(this);
                // TraceLog(LOG_DEBUG, "Deleting particle");
            }
            // std::string str = "Deleted particle with compID: " + std::to_string(componentID);
            // TraceLog(LOG_DEBUG, str.c_str());
        }

        // Check if delay is up. If so, continue to the rest of the function
        // Check if lifetime is up or out of screen. If so, delete yourself
        // Otherwise, draw
        void Update() override
        {
            float dT = GetFrameTime();
            if ((delay -= dT) > 0)
                return;

            if (lifetime < 0 || IsOutsideScreen())
                if (!owner)
                    delete this;
                else
                    CleanUp();

            partTexture->Draw(position, rotation, scale, tint);
            lifetime -= dT;
            position += vel * dT;
            rotation += dT * rotsPerSec;
        }

        // Particles need to be destroyed in a few cases
        //      1. If it went offscreen, since our camera is static
        //      2. If it's past its lifetime

        // The boolean is just to indicate whether we care about lifetime or not
        void CleanUp()
        {
            BallTrail *ownerBT;
            if ((ownerBT = TUtils::GetTypePtr<BallTrail>(owner)) != nullptr)
            {
                auto it = std::find(ownerBT->active.begin(), ownerBT->active.end(), this);
                if (it != ownerBT->active.end())
                {
                    ownerBT->inactive.push(this);
                    ownerBT->active.erase(it);
                }
            }
        }

        bool IsOutsideScreen()
        {
            return (position.x < -DeleteThreshold ||
                    position.y < -DeleteThreshold ||
                    position.x > GetScreenWidth() + DeleteThreshold ||
                    position.y > GetScreenHeight() + DeleteThreshold);
        }
    };
} // namespace pong
