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
    System::~System() {}

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
        // root->PrintDebug(0);
        BeginBlendMode(BLEND_ALPHA);
        root->Draw(raylib::Color(0, 228, 48, 50), raylib::Color::Red());
        EndBlendMode();

        // Naive collision detection, left for checking performance gains
        // for (auto &&colA : systemComponents)
        // {
        //     for (auto &&colB : systemComponents)
        //     {
        //         // Not the same object
        //         if (colA != colB)
        //         {
        //             if (colA->CheckCollision(colB))
        //             {
        //                 colA->OnCollision(colB);
        //                 colB->OnCollision(colA);
        //             }
        //             else
        //             {
        //                 colA->ClearCollision(colB);
        //                 colB->ClearCollision(colA);
        //             }
        //         }
        //     }
        // }

        for (auto &&compA : systemComponents)
        {
            BaseCollision *colA = TUtils::GetTypePtr<BaseCollision>(compA);

            // TODO: Shouldn't need this too!
            if (!colA->currentNode || !colA->currentNode->objsInNode)
                continue;

            // If we're not near anything, clear the colliding list
            if (colA->currentNode->objsInNode->size() == 1 &&
                (*colA->currentNode->objsInNode)[0] == colA)
                colA->collidingWith.clear();

            for (auto &&colB : *colA->currentNode->objsInNode)
            {
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
            delete root;
            root = nullptr;
        }
        RectCollision *rootColl = new RectCollision(0, 0, GetScreenWidth(), GetScreenHeight());
        root = new QuadTree(rootColl, nullptr, &systemComponents);
    }
} // namespace pong
