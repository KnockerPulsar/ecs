#pragma once
#include "IScene.h"
#include "Utils.h"
#include <string>
#include "../include/raylib-cpp/raylib-cpp.hpp"

namespace pong
{
    class MainMenu : public IScene
    {
    private:
        float titleSize = 0.5,
              fontSize = 60,
              animSpeed = 1,
              playQuitSeperation = 40;
        std::string titleText, playText, quitText;
        raylib::Color titleColor;
        bool playPressed = false, quitPressed = false;

    public:
        MainMenu(std::string titleText, std::string playText, std::string quitText,
                 raylib::Color titleColor, float titleSize, float fontSize, float animSpeed, float playQuitSeperation)

            : titleText(titleText), playText(playText), quitText(quitText), titleColor(titleColor),
              titleSize(titleSize), fontSize(fontSize), animSpeed(animSpeed), playQuitSeperation(playQuitSeperation)
        {
            sceneWeb.emplace_back(std::bind(&MainMenu::Exit, this), nullptr);
        }

        virtual void Update() override
        {
            CheckKeys();
            Animate();
            int screenWidth = GetScreenWidth(), screenHeight = GetScreenHeight();
            auto [xTitle, yTitle] = Utils::CenterText(titleText, screenWidth / 2, screenHeight / 4, fontSize * titleSize);
            DrawText(titleText.c_str(), xTitle, yTitle, fontSize * titleSize, titleColor);

            auto [xPlay, yPlay] = Utils::CenterText(playText, screenWidth / 2, screenHeight / 2, fontSize);
            DrawText(playText.c_str(), xPlay, yPlay, fontSize, raylib::Color::Green());

            auto [xQuit, yQuit] = Utils::CenterText(quitText, screenWidth / 2, screenHeight / 2 + playQuitSeperation, fontSize);
            DrawText(quitText.c_str(), xQuit, yQuit , fontSize, raylib::Color::Red());
        }

        void Animate()
        {
            float dT = GetFrameTime();
            static int dir = 1;

            titleSize += dir * dT * animSpeed;
            if (titleSize > 1)
                dir = -1;
            else if (titleSize < 0.5)
                dir = 1;
        }

        void CheckKeys()
        {
            switch (GetKeyPressed())
            {
            case KEY_ENTER:
                playPressed = true;
                break;
            case KEY_Q:
                quitPressed = true;
                break;

            default:
                break;
            }
        }
        bool GoToMainGame() { return playPressed; }
        bool Exit() { return quitPressed; }
    };
}