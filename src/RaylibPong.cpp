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

// TODO: Instead of remaking the whole quadtree for a mostly static scene, we can mark each collider as "static" or
// "dynamic", then we only need to rebuild the nodes for dynamic colliders

// TODO: Code clean up

// TODO: More TODO's
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
  float offset = 10;
  pong::Entity lPaddle(PaddleSize.x, screenHeight / 2 - PaddleSize.y / 2),
      rPaddle(screenWidth - PaddleSize.x * 2, screenHeight / 2 - PaddleSize.y / 2),
      ball((float)(screenWidth / 2), (float)(screenHeight / 2)),
      tWall(0, offset), 
      bWall(0, screenHeight - offset),
      lNet(offset, 0),
      rNet((float)(screenWidth - offset), 0);

  // Positions, TODO: Maybe move this into the entity? Since most components follow their entity
  // Can have seperate positions for components that require that

  // Resources
  raylib::Texture2D wTri("data/particle_triangle_white.png");

  // Main logic components
  pong::Paddle lPaddleComp(&lPaddle.position, PaddleSize, PaddleSpeed, 1, BLUE);
  pong::Paddle rPaddleComp(&rPaddle.position, PaddleSize, PaddleSpeed, 2, RED);
  pong::Ball ballComp(&wTri, &ball.position, ballRadius, ballSpeed, ballSpeed);
  pong::Wall tWallComp, bWallComp;
  pong::ScoreDisplay lScore(scoreSize, &win), rScore(scoreSize, &win);

  // Collisions
  // TODO: Make the collisions a bit taller than the paddles to account
  // for corner collisions
  pong::RectCollision lInitColl = pong::RectCollision(
      &lPaddle.position, PaddleSize.x / 2, PaddleSize.y);
  pong ::RectCollision rInitColl = pong::RectCollision(
      &rPaddle.position, PaddleSize.x / 2, PaddleSize.y);
  pong::BallCollision ballInitColl = pong::BallCollision(&ball.position, ballRadius);

  pong::RectCollision tWallInitColl(&tWall.position, screenWidth, 5);
  pong::RectCollision bWallInitColl(&bWall.position, screenWidth, 5);
  pong::RectCollision lNetInitColl(&lNet.position, 5, screenHeight);
  pong::RectCollision rNetInitColl(&rNet.position, 5, screenHeight);

  // Upcasting all collisions to their parent components
  pong::BaseCollision *lRectColl = &lInitColl;
  pong::BaseCollision *rRectColl = &rInitColl;
  pong::BaseCollision *ballColl = &ballInitColl;
  pong::BaseCollision *tWallColl = &tWallInitColl;
  pong::BaseCollision *bWallColl = &bWallInitColl;
  pong::BaseCollision *lNetColl = &lNetInitColl;
  pong::BaseCollision *rNetColl = &rNetInitColl;

  pong::Net lNetComp(rPaddle.entityID), rNetComp(lPaddle.entityID);
  pong::BallTrail bTrail(ball.entityID, 100, wTri, &ball.position, lPaddleComp.BoxColor, rPaddleComp.BoxColor);

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

  delete pong::Entity::entities;
  // win.Close(); // Causes an excpetion with the texture for some reason

  return 0;
}
