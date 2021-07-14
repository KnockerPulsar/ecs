#pragma once
#include <vector>
#include "src/Random.cpp"
#include "src/components/Component.h"
#include "src/components/BaseCollision.h"
#include "src/components/BallCollision.h"
#include "src/components/RectCollision.h"
#include "src/components/Paddle.h"
#include "src/components/Ball.cpp"

class Ball;
namespace pong
{
    class System
    {
    private:
        std::vector<Component *> systemComponents;
        // https://stackoverflow.com/questions/43944112/c-function-pointer-assignment-cannot-convert-types-inside-a-class
        std::function<void(pong::System *)> updateSystem;

    public:
        System()
        {
        }

        // TODO: Use enable_if and SFINAE to clean up this mess
        System(Paddle *systemComponent)
        {
            updateSystem = &System::UpdateIndependent;
        }

        System(Ball *systemComponent)
        {
            updateSystem = &System::UpdateIndependent;
        }

        System(pong::BaseCollision *systemComponent)
        {
            updateSystem = &System::UpdateCollision;
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
        void UpdateCollision()
        {
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
