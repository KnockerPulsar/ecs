#pragma once
#include <queue>
#include "Component.h"
#include "Ball.h"
#include "../Utils.h"
#include <algorithm>

namespace pong
{
    class Particle;

    // Draws a bunch of particles behind the ball
    // Uses a particle pool instead of continuously creating and
    // destroying them
    class BallTrail : public Component
    {
    private:
        int numParticles, // Size of particle pool
            ballRef;      // The entity ID of the ball
        raylib::Vector2 *ballPos;
        raylib::Color left, right;
        std::vector<Particle *> *parts; // The pool of particles, created dynamically at runtime
        Ball *ball = nullptr;                     // A pointer to the ball this trail is attached to, initialized in Start()

    public:
        std::vector<Particle *> active;  // Particles currently being drawn
        std::queue<Particle *> inactive; // Particles waiting to be dispatched

        BallTrail(int bRef, int numParts, raylib::Texture2D &particleTexture, raylib::Color l, raylib::Color r);

        ~BallTrail();

        // Goes through all inactive particles and dispatches them into the active list
        // Each starts with a random lifetime and a position near the ball
        // The particle's position is also opposite of that of the ball's with some randomness
        // Particles are colored differently depending on whether the ball is in the left or right half

        // Then, the active list calls Update for each particle and checks for clean up

        // I think a cleaner approach would be to add and remove the particles from the particle rendering system
        // depending on their age, but that would incur an overhead due to finding and removing each particle in O(n)

        void Update() override;

        void Start() override;
    };
} // namespace pong
