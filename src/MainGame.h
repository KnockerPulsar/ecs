#pragma once
#include "IScene.h"
#include "raylib.h"
#include "../include/raylib-cpp/raylib-cpp.hpp"

#include <unordered_map>
#include <vector>
#include <string>
#include "components/Paddle.h"
#include "components/Ball.h"
#include "System.h"
#include "Entity.h"
#include "components/Component.h"
#include "components/BaseCollision.h"
#include "components/BallCollision.h"
#include "components/RectCollision.h"
#include "components/ScoreDisplay.h"
#include "components/Net.h"
#include "components/BallTrail.h"
#include "Event.h"

namespace pong
{
    class MainGame : public IScene
    {
    public:
        int maxScore = -1, winnerNum, winningScore;
        raylib::Color winnerColor;
        bool inited = false;

        MainGame(int winningScore) : winningScore(winningScore) {}

        void Update() override
        {

            DrawFPS(20, 20);

            float deltaTime = GetFrameTime();

            // Game loop (logic, collision, particles, drawing)
            for (auto &&system : systems)
                system.second->Update();
        }

        void Start(IScene *prevScene) override
        {
            if (inited)
                return;

            // Common paddle variables
            raylib::Vector2 PaddleSize{15, 100};
            float PaddleSpeed = 1000, ballRadius = 10, ballSpeed = 512,
                  screenWidth = GetScreenWidth(), screenHeight = GetScreenHeight();

            int scoreSize = 90;

            // Left and right paddle inits
            // ============================================================================
            // Entity creation

            float offset = 10;
            static pong::Entity lPaddle(PaddleSize.x, screenHeight / 2 - PaddleSize.y / 2),
                rPaddle(screenWidth - PaddleSize.x * 2, screenHeight / 2 - PaddleSize.y / 2),
                ball((float)(screenWidth / 2), (float)(screenHeight / 2)),
                tWall(0, offset),
                bWall(0, screenHeight - offset),
                lNet(offset, 0),
                rNet((float)(screenWidth - offset), 0);

            // Resources
            static raylib::Texture2D wTri("data/particle_triangle_white.png");

            // Main logic components
            static pong::Paddle rPaddleComp(PaddleSize, PaddleSpeed, 2, RED);
            static pong::Paddle lPaddleComp(PaddleSize, PaddleSpeed, 1, BLUE);
            static pong::Ball ballComp(&wTri, ballRadius, ballSpeed, ballSpeed, (IScene *)this);
            static pong::Wall tWallComp, bWallComp;
            static pong::ScoreDisplay lScore(scoreSize), rScore(scoreSize);

            // Collisions
            static pong::RectCollision lInitColl = pong::RectCollision(
                PaddleSize.x / 2, PaddleSize.y);
            static pong ::RectCollision rInitColl = pong::RectCollision(
                PaddleSize.x / 2, PaddleSize.y);
            static pong::BallCollision ballInitColl = pong::BallCollision(ballRadius);

            static pong::RectCollision tWallInitColl(screenWidth, 5);
            static pong::RectCollision bWallInitColl(screenWidth, 5);
            static pong::RectCollision lNetInitColl(5, screenHeight);
            static pong::RectCollision rNetInitColl(5, screenHeight);

            // Upcasting all collisions to their parent components
            static pong::BaseCollision *lRectColl = &lInitColl;
            static pong::BaseCollision *rRectColl = &rInitColl;
            static pong::BaseCollision *ballColl = &ballInitColl;
            static pong::BaseCollision *tWallColl = &tWallInitColl;
            static pong::BaseCollision *bWallColl = &bWallInitColl;
            static pong::BaseCollision *lNetColl = &lNetInitColl;
            static pong::BaseCollision *rNetColl = &rNetInitColl;

            static pong::Net lNetComp(rPaddle.entityID), rNetComp(lPaddle.entityID);
            static pong::BallTrail bTrail(ball.entityID, 100, wTri, lPaddleComp.BoxColor, rPaddleComp.BoxColor);

            // Adding the components to the paddle entities
            // Note that this also automatically adds the components to their respective systems
            // Without this step, the components will not work
            lPaddle.AddComponent(&lPaddleComp);
            lPaddle.AddComponent(&lScore);
            lPaddle.AddComponent(lRectColl);

            rPaddle.AddComponent(&rPaddleComp);
            rPaddle.AddComponent(rRectColl);
            rPaddle.AddComponent(&rScore);

            ball.AddComponent(&ballComp);
            ball.AddComponent(ballColl);
            ball.AddComponent(&bTrail);

            tWall.AddComponent(tWallColl);
            tWall.AddComponent(&tWallComp);

            bWall.AddComponent(bWallColl);
            bWall.AddComponent(&bWallComp);

            lNet.AddComponent(lNetColl);
            lNet.AddComponent(&lNetComp);

            rNet.AddComponent(rNetColl);
            rNet.AddComponent(&rNetComp);

            // Used to test naive VS quad tree collision
            //  GenerateBalls(100, screenWidth, screenHeight, wTri, ballRadius, ballSpeed, lPaddleComp, rPaddleComp);

            // Starting tasks for each component
            for (auto &&system : systems)
                system.second->Start();

            inited = true;
        }

        void CleanUp(IScene *nextScene) override
        {
            Reset();
        }

        void GenerateBalls(int ballCount, int screenWidth, int screenHeight, raylib::Texture2D &wTri, float ballRadius, float ballSpeed, pong::Paddle &lPaddleComp, pong::Paddle &rPaddleComp)
        {
            std::vector<pong::Entity *> balls(ballCount);
            std::vector<pong::Ball *> ballComps(ballCount);
            std::vector<pong::BallCollision *> ballInitColls(ballCount);
            std::vector<pong::BaseCollision *> ballColls(ballCount);
            std::vector<pong::BallTrail *> ballTrails(ballCount);

            for (int i = 0; i < ballCount; i++)
            {
                balls[i] = new pong::Entity((float)(screenWidth / 2) + pong::Utils::GetRand(-50, 50),
                                            (float)(screenHeight / 2) + pong::Utils::GetRand(-50, 50));

                ballComps[i] = new pong::Ball(&wTri, ballRadius, ballSpeed, ballSpeed, this);

                ballInitColls[i] = new pong::BallCollision(ballRadius);

                ballColls[i] = ballInitColls[i];

                ballTrails[i] = new pong::BallTrail(balls[i]->entityID, 100, wTri, lPaddleComp.BoxColor, rPaddleComp.BoxColor);

                balls[i]->AddComponent(ballComps[i]);

                balls[i]->AddComponent(ballColls[i]);

                balls[i]->AddComponent(ballTrails[i]);
            }
        }

        void Reset()
        {
            maxScore = -1;
            for (auto &&entity : Game::currScene->entites)
            {
                entity.second->Reset();
            }
        }
    };
}