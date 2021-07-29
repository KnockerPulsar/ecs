#include <vector>
#include <algorithm>
#include <functional>
#include "components/Component.h"
#include "components/Particle.h"
#include "TUtils.h"

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

    System::~System()
    {
        if (root)
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
            if (comp->enabled)
                comp->Update();
        }
    }

    // Tests every object with all others
    // FIXME: Need something similar to continuous collision detection
    // Might require moving velocity and collision detection together?
    void System::UpdateCollision()
    {
        UpdateIndependent(); // For debugging, shows colliders

        // NaiveCollision();
        QuadTreeCollision();
    }

    void System::UpdateBlendedParticles()
    {
        BeginBlendMode(BLEND_ALPHA);

        // For some reason, putting a forrange loop here causes
        // the system to try and execute Update() on a component
        // does not exist?
        for (int i = 0; i < systemComponents.size(); i++)
        {
            if (systemComponents[i]->enabled)
                systemComponents[i]->Update();
        }

        EndBlendMode();
    }

    void System::NaiveCollision()
    {
        for (auto &&colA : systemComponents)
        {
            if (!colA->enabled)
                return;
            for (auto &&colB : systemComponents)
            {
                if (colA != colB && colB->enabled)
                {
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

    void System::QuadTreeCollision()
    {
        BuildQuadTree();
        root->Draw(raylib::Color(0, 228, 48, 50), raylib::Color::Red());

        for (auto &&compA : systemComponents)
        {
            if (!compA->enabled)
                return;

            BaseCollision *colA = TUtils::GetTypePtr<BaseCollision>(compA);
            for (auto &&currentNode : colA->currentNodes)
            {
                // // If we're not near anything, clear the colliding list
                // if (currentNode->contained.size() == 1 &&
                //     currentNode->contained[0] == colA)
                //     {colA->collidingWith.clear();}

                for (auto &&colB : currentNode->contained)
                {
                    if (!colB->enabled)
                        return;
                    // If we're checking an object against itself
                    if (colA == colB)
                        continue;

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

    void System::AddComponentIndep(Component *comp)
    {
        if (!comp)
            return;
        systems[comp->tag]->AddComponent(comp);
    }

    void System::RemoveComponentIndep(Component *comp)
    {
        if (!comp)
            return;

        systems[comp->tag]->RemoveComponent(comp);
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
        RectCollision rootColl = RectCollision(GetScreenWidth(), GetScreenHeight());
        rootColl.position = vec;

        rootColl.entityID = 6969;

        root = new QuadTree(rootColl, 2);
        for (auto &&comp : systemComponents)
        {
            root->Insert(comp, 0);
        }
    }
} // namespace pong
