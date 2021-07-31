#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include "Event.h"
#include "Game.h"
#include "Tags.h"
#include "System.h"

namespace pong
{
    class Entity;

    class IScene
    {
    public:
        std::unordered_map<int, Entity *> entites; // Entities keyed by their entity ID
        std::vector<Event *> events;
        std::unordered_map<pong::tags, pong::System *, pong::TagsHashClass> systems;

        // Only one transition should be active at any time
        // How to enforce this?
        std::vector<std::pair<std::function<bool()>, IScene *>> sceneWeb;

        virtual void Update(){};                   // Called every frame
        virtual void Start(IScene *prevScene){};   // Called when transitioning into this scene
        virtual void CleanUp(IScene *nextScene){}; // Called when transitioning from this scene

        // Loops over all transitions and checks if they return true
        // Returns the first scene with a true transition
        IScene *CheckTransitions()
        {
            for (auto &&edge : sceneWeb)
            {
                IScene *nextScene = edge.second;
                bool shouldTransition = edge.first();

                if (shouldTransition)
                {
                    this->CleanUp(nextScene);
                    Game::currScene = nextScene;
                    if (nextScene)
                        nextScene->Start(this);
                    return nextScene;
                }
            }

            // No transitions found, stay on the same scene
            return this;
        }

        void AddTransition(std::function<bool()> transitionCheck, IScene *scene)
        {
            sceneWeb.emplace_back(transitionCheck, scene);
        }

        void CheckSceneEvents(float dT)
        {
            for (auto &&event : events)
                event->Tick(dT);
        }

        void UpdateSystems()
        {
            for (auto &&system : systems)
                system.second->Update();
        }

        template <typename func, typename... args>
        void AddEvent(float delay, func &&fun, args... arguments)
        {
            Event *ev = new Event();
            ev->fn = std::bind(fun, arguments...);
            ev->delay = delay;
            events.push_back(ev);
        }

        template <typename func>
        void AddEvent(float delay, func &&fun)
        {
            Event *ev = new Event();
            ev->fn = fun;
            ev->delay = delay;
            events.push_back(ev);
        }
    };

}