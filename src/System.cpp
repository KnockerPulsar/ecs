#include <vector>
#include <algorithm>
#include <functional>
#include "components/Component.h"
#include "components/Particle.h"
#include "TUtils.h"

// TODO: Maybe compose the update function of multiple ones to make systems more flexible?
// Update() only loops through the vector of functions and executes them
#include "System.h"

namespace pong
{

    std::unordered_map<pong::tags, pong::System *, pong::TagsHashClass> System::systems;

    System::System(tags compTag)
    {
        switch (compTag)
        {
        case tags::indep:
            updateSystem = &System::UpdateIndependent;
            break;

        case tags::coll:
            updateSystem = &System::UpdateCollision;
            break;

        case tags::particle:
            updateSystem = &System::UpdateBlendedParticles;
            break;

        default:
            break;
        }
    }
    System::~System() {
        delete root; 
    }

    // TODO: Use enable_if and SFINAE to make this look better
    // Compile UpdateIndependent as Update for certain types
    // And compile UpdateCollision as Update for Collision systems
    // There's probably an even better solution to this...
    void System::Update()
    {
        this->updateSystem(this);
    }

    // If every component is independent of others
    // Drawing systems or player controllers for example
    void System::UpdateIndependent()
    {
        for (auto &&comp : systemComponents)
        {
            comp->Update();
        }
    }

    // Tests every object with all others
    // FIXME: Need something similar to continuous collision detection
    // Might require moving velocity and collision detection together?
    void System::UpdateCollision()
    {
        BuildQuadTree();
        UpdateIndependent(); // For debugging

        root->Draw(raylib::Color(0, 228, 48, 50), raylib::Color::Red());

        for (auto &&compA : systemComponents)
        {
            BaseCollision *colA = TUtils::GetTypePtr<BaseCollision>(compA);
            for (auto &&currentNode : colA->currentNodes)
            {
                // // If we're not near anything, clear the colliding list
                // if (currentNode->contained.size() == 1 &&
                //     currentNode->contained[0] == colA)
                //     {colA->collidingWith.clear();}

                for (auto &&colB : currentNode->contained)
                {
                    // If we're checking an object against itself
                    if (colA == colB)
                        continue;

                    BallCollision *b1 = TUtils::GetTypePtr<BallCollision>(colA);
                    RectCollision *r1 = TUtils::GetTypePtr<RectCollision>(colB);

                    RectCollision *r2 = TUtils::GetTypePtr<RectCollision>(colA);
                    BallCollision *b2 = TUtils::GetTypePtr<BallCollision>(colB);

                    // BallCollision* b2 = colB->GetTypePtr<BallCollision>();
                    // RectCollision* r2 = colA->GetTypePtr<RectCollision>();

                    if (colA->CheckCollision(colB))
                    {
                        colA->OnCollision(colB);
                        colB->OnCollision(colA);
                    }
                    else
                    {
                        colA->ClearCollision(colB);
                        colB->ClearCollision(colA);
                    }
                }
            }
        }
    }
    void System::UpdateBlendedParticles()
    {
        BeginBlendMode(BLEND_ALPHA);
        for (int i = 0; i < systemComponents.size(); i++)
        {
            Component *comp = systemComponents[i];
            comp->Update();
        }

        EndBlendMode();
    }

    void System::Start()
    {
        for (auto &&comp : systemComponents)
        {
            comp->Start();
        }
    }
    void System::AddComponent(Component *component)
    {
        // Might be useful for initializations on add or something
        systemComponents.push_back(component);
    }
    void System::RemoveComponent(Component *comp)
    {
        auto it = std::find(systemComponents.begin(), systemComponents.end(), comp);
        if (it != systemComponents.end())
            systemComponents.erase(it);
        else
            TraceLog(LOG_DEBUG, ("Shouldn't be here! " + std::to_string(comp->componentID)).c_str());
    }

    void System::BuildQuadTree()
    {
        if (root)
        {
            // delete root->collision.position;            
            delete root;
            root = nullptr;
            for (auto &&comp : systemComponents)
            {
                TUtils::GetTypePtr<BaseCollision>(comp)->currentNodes.clear();
            }
        }
        raylib::Vector2 *vec = new raylib::Vector2();
        RectCollision rootColl = RectCollision(vec, GetScreenWidth(), GetScreenHeight());
        rootColl.entityID = 6969;

        root = new QuadTree(rootColl, 2);
        for (auto &&comp : systemComponents)
        {
            root->Insert(comp, 0);
        }
    }
} // namespace pong
