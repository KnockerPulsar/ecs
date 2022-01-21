#pragma once
#include <vector>
#include <string>
#include "../include/raylib-cpp/include/raylib-cpp.hpp"
#include "Event.h"

namespace pong
{
    class IScene;

    // The glue that holds the game together
    // Only one instance should be active at a time
    class Game
    {
    private:
        static std::vector<IScene *> scenes;
        static std::vector<Event *> events;

    public:
        static IScene *currScene;

        Game(int windowWidth, int windowHeight, std::string windowTitle, int targetFPS = 60, TraceLogLevel logLevel = LOG_INFO);

        Game *AddScene(IScene *scene);

        // Currently just calls currentScene->Start(nullptr)
        // Might be useful for more initializations
        void Init();

        void Run();

        void CheckGameEvents(float dT);


        template <typename func, typename... args>
        static void AddEvent(float delay, func &&fun, args... arguments)
        {
            Event *ev = new Event();
            ev->fn = std::bind(fun, arguments...);
            ev->delay = delay;
            Game::events.push_back(ev);
        }

        template <typename func>
        static void AddEvent(float delay, func &&fun)
        {
            Event *ev = new Event();
            ev->fn = fun;
            ev->delay = delay;
            Game::events.push_back(ev);
        }
    };
}