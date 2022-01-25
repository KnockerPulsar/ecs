#include "./MainMenu.h"
#include "./MainGame.h"
#include "./WinScreen.h"
#include "Game.h"

#ifndef MAX_FPS
  #define MAX_FPS 60
#endif

/*
  ecs System:
  Instead of creating specific classes from each specific entity, we'd create a basic entity class.
  It would contain mainly a container for all its components (maybe a vector of components pointers perhaps)
  Then we'd create multiple components for the game logic. To add them to an entity, we'd just add them to the vector
  (Or we'd use a function in case we have other internal lists we need to maintain, such as a physics list for example)
*/


int main()
{
  int screenWidth = 1280, screenHeight = 720;

  // Scenes creation
  pong::MainMenu menu(
      "RaylibPong!",
      "Press <Enter> to play",
      "Press <Q> to quit",
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
  pong::Game gameInstance(screenWidth, screenHeight, "RaylibPong!", MAX_FPS, LOG_ALL);

  // CHAINING BABYYYYYYYYYYYYYYY
  // Adds scenes, with each having its transitions
  gameInstance.AddScene(&menu)->AddScene(&game)->AddScene(&win);

  // Game initialization and main loop
  gameInstance.Init();
  gameInstance.Run();

  // CloseWindow(); // Crashes on exit when de-allocating textures for some reason
}