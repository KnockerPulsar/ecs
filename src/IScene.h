#pragma once
#include <vector>
#include <functional>

namespace pong
{
    class IScene
    {
    public:
        // Only one transition should be active at any time
        // How to enforce this?
        std::vector<std::pair<std::function<bool()>, IScene *>> sceneWeb;

        virtual void Update(){};  // Called every frame
        virtual void Start(){};   // Called when transitioning into this scene
        virtual void CleanUp(){}; // Called when transitioning from this scene

        // Loops over all transitions and checks if they return true
        // Returns the first scene with a true transition
        IScene *CheckTransitions()
        {
            for (auto &&edge : sceneWeb)
            {
                if (edge.first())
                {
                    this->CleanUp();
                    if (edge.second)
                        edge.second->Start();
                    return edge.second;
                }
            }

            // No transitions found, stay on the same scene
            return this;
        }

        void AddTransition(std::function<bool()> transitionCheck, IScene *scene)
        {
            sceneWeb.emplace_back(transitionCheck, scene);
        }
    };

}