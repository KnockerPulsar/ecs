#pragma once
#include <vector>
#include <algorithm>
#include <functional>
#include "components/Ball.h"
#include "components/Component.h"
#include "components/BaseCollision.h"
#include "QuadTree.h"

class Ball;

namespace pong
{
    class Paddle;

    class System
    {
    public:
        // Stores this system's components
        std::vector<Component *> systemComponents;

        // https://stackoverflow.com/questions/43944112/c-function-pointer-assignment-cannot-convert-types-inside-a-class
        // Points to this system's specific update function
        std::function<void(System *)> updateSystem;

        // For collision systems only, points to the root of the quad tree
        QuadTree *root = nullptr;

        // Sets the update function corresponding to each system type
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

        // Naive collision detection algorithm, O(n^2)
        void NaiveCollision();

        // Quad-tree assisted collision detection, O(nlogn)
        void QuadTreeCollision();

        // To initialize this system's component's
        // Calls component->Update()
        // Might be useful for initializations not possible in the constructor
        void Start();

        // Adds a component to a system
        void AddComponent(Component *component);

        // Removes it
        void RemoveComponent(Component *comp);

        // For components that don't really require an entity (AKA particle effects)
        static void AddComponentIndep(Component *comp);
        static void RemoveComponentIndep(Component *comp);

        // Recursively builds the quad tree
        // Note that only the leaves have data
        void BuildQuadTree();
    };

} // namespace pong
