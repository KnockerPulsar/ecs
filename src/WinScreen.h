#pragma oncec
#include "../vendor/raylib-cpp/raylib-cpp.hpp"
#include <string>
#include "Utils.h"
#include "IScene.h"
#include "MainGame.h"

namespace pong
{
    class WinScreen : public IScene
    {
    private:
        std::string message, playMes = "Press <Enter> to replay", quitMes = "Press <Q> to quit";
        raylib::Color playerColor;
        int winnerNum, lineSeperation = 60;

    public: 
        bool replay = false, quit = false;

        void Update() override
        {
            CheckKeys();

            int screenWidth = GetScreenWidth(), screenHeight = GetScreenHeight();
            auto [xMes, yMes] = Utils::CenterText(message, screenWidth / 2, screenHeight / 2, 80);
            DrawText(message.c_str(), xMes, yMes, 80, playerColor);

            auto [xPlay, yPlay] = Utils::CenterText(playMes, screenWidth / 2, screenHeight / 2 + lineSeperation, 40);
            DrawText(playMes.c_str(), xPlay, yPlay, 40, raylib::Color::Green());

            auto [xQuit, yQuit] = Utils::CenterText(quitMes, screenWidth / 2, screenHeight / 2 + 2 * lineSeperation, 40);
            DrawText(quitMes.c_str(), xQuit, yQuit, 40, raylib::Color::Red());
        }

        void CheckKeys()
        {
            switch (GetKeyPressed())
            {
            case KEY_ENTER:
                replay = true;
                break;
            case KEY_Q:
                quit = true;
                break;
            default:
                break;
            }
        }

        void Start(IScene *prevScene) override
        {
            MainGame *game;
            if (game = dynamic_cast<MainGame *>(prevScene))
            {
                this->winnerNum = game->winnerNum;
                this->playerColor = game->winnerColor;
                this->message = "Player " + std::to_string(winnerNum) + " wins!";
            }
        }

        void CleanUp(IScene* nextScene)
        {
            quit = false;
            replay = false;
        }
    };
}