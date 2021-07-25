#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include "components/Ball.h"
#include "components/Component.h"
#include "components/BaseCollision.h"
#include "QuadTree.h"


// TODO: Maybe compose the update function of multiple ones to make systems more flexible?
// Update() only loops through the vector of functions and executes them

class Ball;

namespace pong
{
    class Paddle;

    class System
    {
    private:
    public:
        static std::unordered_map<pong::tags, pong::System *, pong::TagsHashClass> systems;
        std::vector<Component *> systemComponents;

        // https://stackoverflow.com/questions/43944112/c-function-pointer-assignment-cannot-convert-types-inside-a-class
        std::function<void(System *)> updateSystem;

        QuadTree* root = nullptr;

        System(tags compTag);
        ~System();

        // TODO: Use enable_if and SFINAE to make this look better
        // Compile UpdateIndependent as Update for certain types
        // And compile UpdateCollision as Update for Collision systems
        // There's probably an even better solution to this...
        void Update();

        // If every component is independent of others
        // Drawing systems or player controllers for example
        void UpdateIndependent();

        // Tests every object with all others
        void UpdateCollision();

        // Renders blended particles
        void UpdateBlendedParticles();

        void Start();
        void AddComponent(Component *component);
        void RemoveComponent(Component *comp);

        void BuildQuadTree();
    };

} // namespace pong
