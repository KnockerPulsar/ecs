#include "Game.h"
#include "IScene.h"

namespace pong
{
    // Static variable initializations
    IScene *Game::currScene;

    std::vector<IScene *> Game::scenes;
    std::vector<Event *> Game::events;

    Game::Game(int windowWidth, int windowHeight, std::string windowTitle, int targetFPS, TraceLogLevel logLevel)
    {
        InitWindow(windowWidth, windowHeight, windowTitle.c_str());
        SetTargetFPS(targetFPS);
        SetTraceLogLevel(logLevel);
    }

    Game *Game::AddScene(IScene *scene)
    {
        if (scenes.size() == 0)
            currScene = scene;
        scenes.push_back(scene);

        return this;
    }
    void Game::Init()
    {
        if (Game::currScene)
            Game::currScene->Start(nullptr);
    }

    void Game::Run()
    {
        while (!WindowShouldClose())
        {
            float dT = GetFrameTime();

            ClearBackground(BLACK);
            BeginDrawing();

            this->CheckGameEvents(dT);
            Game::currScene = Game::currScene->CheckTransitions();

            // If currentScene is nullptr, exit the game
            if (!(Game::currScene = Game::currScene->CheckTransitions()))
                break;

            
            Game::currScene->Update();
            
            Game::currScene->CheckSceneEvents(dT);

            EndDrawing();
        }
    }

    void Game::CheckGameEvents(float dT)
    {
        for (auto &&event : Game::events)
            event->Tick(dT);
    }
}