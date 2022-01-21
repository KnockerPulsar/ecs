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

        // Given a fucntion (preferably a lambda) and a scene
        // Calls back this function every frame to check for scene transitions
        void AddTransition(std::function<bool()> transitionCheck, IScene *scene)
        {
            sceneWeb.emplace_back(transitionCheck, scene);
        }

        // Loops over the event queue of the current scene
        void CheckSceneEvents(float dT)
        {
            for (auto &&event : events)
                event->Tick(dT);
        }

        // Loops over each system contained in the scene and updates it,
        // which internally loops over the components contained in that system
        void UpdateSystems()
        {
            // C++ 17 destructurings
            // Similar to how python uses tuples
            for (auto &&[tag, system] : systems)
                system->Update();
        }

        // Add an event to the event queue with parameters
        template <typename func, typename... args>
        void AddEvent(float delay, func &&fun, args... arguments)
        {
            Event *ev = new Event(
                delay,
                std::bind(fun, arguments...));
            events.push_back(ev);
        }

        // Add an even to the event queue without parameters
        // fn(void)
        template <typename func>
        void AddEvent(float delay, func &&fun)
        {
            Event *ev = new Event(delay, fun);
            events.push_back(ev);
        }
    };

}