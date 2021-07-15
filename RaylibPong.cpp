#include "raylib.h"
#include <unordered_map>
#include <vector>
#include <string>
#include "src/Paddle.h"
#include "src/Ball.h"
#include "src/System.h"
#include "src/Entity.h"
#include "src/Component.h"
#include "src/BaseCollision.h"
#include "src/RectCollision.h"
#include "src/RectCollision.cpp"
#include "src/BallCollision.cpp"
#include "src/Ball.cpp"
#include "src/Entity.cpp"

// KNOWN BUG: If you hit the ball with the side of the paddle, the ball might get stuck
// inside the paddle;

// TODO: Collision

/*
  ecs System:
  Instead of creating specific classes from each specific entity, we'd create a basic entity class.
  It would contain mainly a container for all its components (maybe a vector of components pointers perhaps)
  Then we'd create multiple components for the game logic. To add them to an entity, we'd just add them to the vector
  (Or we'd use a function in case we have other internal lists we need to maintain, such as a physics list for example)

*/

int main()
{
  const int screenWidth = 1280;
  const int screenHeight = 720;
  InitWindow(screenWidth, screenHeight, "RaylibPong");
  SetTargetFPS(60);
  SetTraceLogLevel(LOG_ALL);

  // System storage creation
  // Each system corresponds to a tag
  std::unordered_map<pong::tags, pong::System *, pong::TagsHashClass> systems;

  // Common paddle variables
  Vector2 PaddleSize{15, 100};
  float PaddleSpeed = 1000;
  float ballRadius = 10;
  float ballSpeed = 512;

  // Left and right paddle inits
  pong::Entity leftPaddle;
  Vector2 lPaddlePos = (Vector2){PaddleSize.x, screenHeight / 2};
  pong::Paddle lPaddleComp(
      &lPaddlePos,
      PaddleSize,
      PaddleSpeed,
      1,
      BLUE);
  pong::RectCollision lInitColl = pong::RectCollision(&lPaddlePos, PaddleSize.x, PaddleSize.y);
  pong::BaseCollision *lRectColl = &lInitColl;

  pong::Entity rightPaddle;
  Vector2 rPaddlePos = (Vector2){screenWidth - PaddleSize.x * 2, screenHeight / 2};
  pong::Paddle rPaddleComp(
      &rPaddlePos,
      PaddleSize,
      PaddleSpeed,
      2,
      RED);
  pong ::RectCollision rInitColl = pong::RectCollision(&rPaddlePos, PaddleSize.x, PaddleSize.y);
  pong::BaseCollision *rRectColl = &rInitColl;

  pong::Entity ball;
  Vector2 ballPos = (Vector2){screenWidth / 2, screenHeight / 2};
  pong::Ball ballComp(
      &ballPos,
      ballRadius,
      ballSpeed / 2,
      ballSpeed * 2);
  pong::BallCollision ballInitColl = pong::BallCollision(&ballPos, ballRadius);
  pong::BaseCollision *ballColl = &ballInitColl;

  // Adding the components to the paddle entities
  // Note that this also automatically adds the components to their respective systems
  // Without this step, the components will not work
  leftPaddle.AddComponent(&systems, &lPaddleComp);
  leftPaddle.AddComponent(&systems, lRectColl);
  rightPaddle.AddComponent(&systems, &rPaddleComp);
  rightPaddle.AddComponent(&systems, rRectColl);
  ball.AddComponent(&systems, &ballComp);
  ball.AddComponent(&systems, ballColl);

  // Starting tasks
  for (auto &&system : systems)
  {
    system.second->Start();
  }
  // Main loop
  while (!WindowShouldClose())
  {
    ClearBackground(BLACK);
    BeginDrawing();
    for (auto &&system : systems)
    {
      system.second->Update();
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
