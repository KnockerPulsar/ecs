#include "src/Includes.h"

// KNOWN BUG: If you hit the ball with the side of the paddle, the ball might get stuck
// inside the paddle;

// TODO: Scale the paddles down
// TODO: COLOR PAD SEGMENTS WHEN HIT
// TODO: Change gamestates to be function ptrs instead of using a switch statement
// TODO: Use an enum or a define for gamestate

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

  // Map the system's component's name to it
  std::map<std::string, pong::System *> systems;

  // Common paddle variables
  Vector2 PaddleSize{15, 100};
  float PaddleSpeed = 1000;
  float ballRadius = 10;
  float ballSpeed = 512;

  // Left and right paddle inits
  pong::Entity leftPaddle;
  Vector2 lPaddlePos = (Vector2){PaddleSize.x, screenHeight / 2};
  Paddle lPaddleComp(
      &lPaddlePos,
      PaddleSize,
      PaddleSpeed,
      1,
      BLUE);

  pong::Entity rightPaddle;
  Vector2 rPaddlePos = (Vector2){screenWidth - PaddleSize.x * 2, screenHeight / 2};
  Paddle rPaddleComp(
      &rPaddlePos,
      PaddleSize,
      PaddleSpeed,
      2,
      RED);

  pong::Entity ball;
  Vector2 ballPos = (Vector2){screenWidth / 2, screenHeight / 2};
  Ball ballComp(
      &ballPos,
      ballRadius,
      ballSpeed / 2,
      ballSpeed * 2);

  // Adding the components to the paddle entities
  // Note that this also automatically adds the components to their respective systems
  // Without this step, the components will not work
  leftPaddle.AddComponent(&systems, &lPaddleComp);
  rightPaddle.AddComponent(&systems, &rPaddleComp);
  ball.AddComponent(&systems, &ballComp);

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
