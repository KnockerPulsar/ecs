#include "./MainMenu.h"
#include "./MainGame.h"
#include "./WinScreen.h"
#include "Game.h"

// FIXME: Iterator invalidation when deleting components or entities while iterating over a vector of them.
// Not really a big issue now.

// FIXME:?? If you hit the ball with the side of the paddle, the ball might get stucks
// inside the paddle;

/*
  ecs System:
  Instead of creating specific classes from each specific entity, we'd create a basic entity class.
  It would contain mainly a container for all its components (maybe a vector of components pointers perhaps)
  Then we'd create multiple components for the game logic. To add them to an entity, we'd just add them to the vector
  (Or we'd use a function in case we have other internal lists we need to maintain, such as a physics list for example)

*/
// TODO: Remove events from the queue/vector on completion

// TODO: Convert the event vector to a queue that's sorted by delay 

// TODO: Drawing all particles to a texture first then displaying that on screen, might be better than lots of drawcalls?

// TODO: 2D layers

// TODO: Implement "ephemeral" and "persistent" scenes

// TODO: Maybe remove system and event checking from the scene code, leaving just the Update() function for custom behaviour?

// TODO: Make adding components to entities at compile time easier (through non-type arguments entt.AddComponents<comp1, comp2,...>())

// TODO: Improve on the "event" system
// Add repeated calls with delay (foo() every 0.5 seconds for example)
// or calls that will be called a certain number of frames/time. (foo() for every frame for 0.5 seconds for example)
// Take into account if the event should repeat for the event queue sorting function so that repeated events are not popped.

// TODO: Instead of remaking the whole quadtree for a mostly static scene, we can mark each collider as "static" or
// "dynamic", then we only need to rebuild the nodes for dynamic colliders

// TODO: Improve the "scene" system

// TODO: More TODO's

int main()
{
  int screenWidth = 1280, screenHeight = 720;

  // Scenes creation
  pong::MainMenu menu("RaylibPong!", "Press <Enter> to play", "Press <Q> to quit",
                      raylib::Color::Magenta(), 0.5, 60, 0.5, 80);

  pong::MainGame game(5);

  // Need to pass the winning player's number and color somehow...
  pong::WinScreen win;

  // Scene transitions
  // Menu     -> game       (on enter pressed)
  // game     -> win screen (on someone winning)
  // winning  -> game       (on replay chosen)
  // win      -> exit       (on quit chosen)
  menu.AddTransition(std::bind(&pong::MainMenu::GoToMainGame, &menu), &game);

  game.AddTransition(
      [&game]
      { return game.maxScore == game.winningScore; },
      &win);

  win.AddTransition(
      [&win]
      { return win.replay; },
      &game);

  win.AddTransition(
      [&win]
      { return win.quit; },
      nullptr);

  // Game instance creation
  pong::Game gameInstance(screenWidth, screenHeight, "RaylibPong!", 0, LOG_ALL);

  // CHAINING BABYYYYYYYYYYYYYYY
  // Adds scenes, with each having its transitions
  gameInstance.AddScene(&menu)->AddScene(&game)->AddScene(&win);

  // Game initialization and main loop
  gameInstance.Init();
  gameInstance.Run();

  // CloseWindow(); // Crashes on exit when de-allocating textures for some reason
}