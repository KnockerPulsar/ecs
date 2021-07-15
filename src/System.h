#pragma once
#include <vector>
#include <functional>
#include "src/Component.h"
#include "src/BaseCollision.h"
#include "src/Ball.h"

// TODO: Maybe compose the update function of multiple ones to make systems more flexible?
// Update() only loops through the vector of functions and executes them

class Ball;

namespace pong
{
    class Paddle;

    class System
    {
    private:
        std::vector<Component *> systemComponents;
        // https://stackoverflow.com/questions/43944112/c-function-pointer-assignment-cannot-convert-types-inside-a-class
        std::function<void(pong::System *)> updateSystem;

    public:
        System(tags compTag)
        {
            switch (compTag)
            {
            case tags::indep:
                updateSystem = &System::UpdateIndependent;

                break;
            case tags::coll:
                updateSystem = &System::UpdateCollision;
                break;

            default:
                break;
            }
        }
        ~System() {}

        // TODO: Use enable_if and SFINAE to make this look better
        // Compile UpdateIndependent as Update for certain types
        // And compile UpdateCollision as Update for Collision systems
        // There's probably an even better solution to this...
        void Update()
        {
            this->updateSystem(this);
        }

        // If every component is independent of others
        // Drawing systems or player controllers for example
        void UpdateIndependent()
        {
            for (auto &&comp : systemComponents)
            {
                comp->Update(&systemComponents);
            }
        }

        // Tests every object with all others
        // TODO: Implement something like OnCollisionEnter(), OnCollisionStay(), OnCollisionExit()
        // The current OnCollision implementation is close to OnCollisionStay()
        void UpdateCollision()
        {
            UpdateIndependent();
            for (auto &&colA : systemComponents)
            {
                for (auto &&colB : systemComponents)
                {
                    // Not the same object
                    if (colA != colB)
                    {
                        if (colA->CheckCollision(colB))
                        {
                            colA->OnCollision(colB);
                            colB->OnCollision(colA);
                        }
                    }
                }
            }
        }

        void Start()
        {
            for (auto &&comp : systemComponents)
            {
                comp->Start();
            }
        }
        void AddComponent(Component *component)
        {
            // Might be useful for initializations on add or something
            systemComponents.push_back(component);
        }

        // // Naive approach, not very good.
        // // GenerateRandomVelocity(0, 0, minSpeed, maxSpeed);
        // // One possible optimization: check for objects a certain distance away (could cause bugs with fast object)
        // void CheckCollision(Component *colA, Component *colB)
        // {

        //     // Check the type of each collider
        //     // For now, either a ball or a rect collision

        //     // Ball collision checks
        //     if (BallCollision *other = dynamic_cast<BallCollision *>(col))
        //     {
        //         // If both objects collided
        //         if (CheckCollisionCircles(*position, radius, *other->position, other->radius))
        //         {
        //         }
        //     }
        //     // Rect collision checks
        //     else if (RectCollision *other = dynamic_cast<RectCollision *>(col))
        //     {
        //     }
        // }
    };
} // namespace pong
