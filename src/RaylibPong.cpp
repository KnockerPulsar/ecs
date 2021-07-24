#include "raylib.h"
#include "../vendor/raylib-cpp/raylib-cpp.hpp"
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
#include "components/Wall.h"
#include "components/ScoreDisplay.h"
#include "components/Net.h"
#include "components/BallTrail.h"

// FIXME:?? If you hit the ball with the side of the paddle, the ball might get stuck
// inside the paddle;

/*
  ecs System:
  Instead of creating specific classes from each specific entity, we'd create a basic entity class.
  It would contain mainly a container for all its components (maybe a vector of components pointers perhaps)
  Then we'd create multiple components for the game logic. To add them to an entity, we'd just add them to the vector
  (Or we'd use a function in case we have other internal lists we need to maintain, such as a physics list for example)

*/

// TODO: Maybe make a delayed function call system?
// Can be implemented through a detached thread (std::thread)
// or with a loop in the main loop, just needs a reference to the queue, we add to it and it executes
// Should have a priority queue of functions sorted descendingly by time remaining

int main()
{
  int screenWidth = 1280, screenHeight = 720;
  raylib::Window win(screenWidth, screenHeight, "Raylib Pong");
  // win.SetTargetFPS(60);
  SetTraceLogLevel(LOG_ALL);

  // Common paddle variables
  raylib::Vector2 PaddleSize{15, 100};
  float PaddleSpeed = 1000;
  float ballRadius = 10;
  float ballSpeed = 512;
  int scoreSize = 90;

  // Left and right paddle inits
  // ============================================================================
  // Entity creation
  pong::Entity lPaddle, rPaddle, ball, tWall, bWall, lNet, rNet;

  // Positions, TODO: Maybe move this into the entity? Since most components follow their entity
  // Can have seperate positions for components that require that
  float offset = 5;
  raylib::Vector2 lPaddlePos = raylib::Vector2(PaddleSize.x, screenHeight / 2 - PaddleSize.y / 2);
  raylib::Vector2 rPaddlePos = raylib::Vector2{screenWidth - PaddleSize.x * 2, screenHeight / 2 - PaddleSize.y / 2};
  raylib::Vector2 ballPos = raylib::Vector2{(float)(screenWidth / 2), (float)(screenHeight / 2)};
  raylib::Vector2 tWallPos = raylib::Vector2(0, offset);
  raylib::Vector2 bWallPos = raylib::Vector2(0, screenHeight - offset);
  raylib::Vector2 lNetPos = {offset, 0};
  raylib::Vector2 rNetPos = {(float)(screenWidth - offset), 0};

  // Resources
  raylib::Texture2D wTri("data/particle_triangle_white.png");

  // Main logic components
  pong::Paddle lPaddleComp(&lPaddlePos, PaddleSize, PaddleSpeed, 1, BLUE);
  pong::Paddle rPaddleComp(&rPaddlePos, PaddleSize, PaddleSpeed, 2, RED);
  pong::Ball ballComp(&wTri, &ballPos, ballRadius, ballSpeed, ballSpeed);
  pong::Wall tWallComp, bWallComp;
  pong::ScoreDisplay lScore(scoreSize, &win), rScore(scoreSize, &win);

  // Collisions
  // TODO: Make the collisions a bit taller than the paddles to account
  // for corner collisions
  pong::RectCollision lInitColl = pong::RectCollision(
      &lPaddlePos, PaddleSize.x / 2, PaddleSize.y);
  pong ::RectCollision rInitColl = pong::RectCollision(
      &rPaddlePos, PaddleSize.x / 2, PaddleSize.y);
  pong::BallCollision ballInitColl = pong::BallCollision(&ballPos, ballRadius);

  pong::RectCollision tWallInitColl(&tWallPos, screenWidth, 5);
  pong::RectCollision bWallInitColl(&bWallPos, screenWidth, 5);
  pong::RectCollision lNetInitColl(&lNetPos, 5, screenHeight);
  pong::RectCollision rNetInitColl(&rNetPos, 5, screenHeight);

  // Upcasting all collisions to their parent components
  pong::BaseCollision *lRectColl = &lInitColl;
  pong::BaseCollision *rRectColl = &rInitColl;
  pong::BaseCollision *ballColl = &ballInitColl;
  pong::BaseCollision *tWallColl = &tWallInitColl;
  pong::BaseCollision *bWallColl = &bWallInitColl;
  pong::BaseCollision *lNetColl = &lNetInitColl;
  pong::BaseCollision *rNetColl = &rNetInitColl;

  pong::Net lNetComp(rPaddle.entityID), rNetComp(lPaddle.entityID);
  pong::BallTrail bTrail(ball.entityID, 100, wTri, &ballPos, lPaddleComp.BoxColor, rPaddleComp.BoxColor);

  // Adding the components to the paddle entities
  // Note that this also automatically adds the components to their respective systems
  // Without this step, the components will not work
  lPaddle.AddComponent(&lPaddleComp);
  lPaddle.AddComponent(lRectColl);
  lPaddle.AddComponent(&lScore);

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

  // Starting tasks for each component
  for (auto &&system : pong::System::systems)
  {
    system.second->Start();
  }

  // Main loop
  while (!WindowShouldClose())
  {
    win.ClearBackground(BLACK);
    win.BeginDrawing();
    for (auto &&system : pong::System::systems)
    {
      system.second->Update();
    }
    win.EndDrawing();
  }

  return 0;
}
