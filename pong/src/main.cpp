#include "defs.h"
#include "ecs.h"

#include "level.h"
#include "levels/common.h"
#include "levels/game_over.h"
#include "levels/main_game.h"
#include "levels/main_menu.h"

#include "raylib.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

int main() {

  InitWindow(800, 800, "ecs-pong");

  ecs::ECS ecs;

  ecs.addGlobalResource(pong::Time{0.0});
  ecs.addGlobalResource(pong::DeltaTime{1. / 60.});
  ecs.addGlobalResource(pong::ScreenWidth(GetScreenWidth()));
  ecs.addGlobalResource(pong::ScreenHeight(GetScreenHeight()));

  ecs.addGlobalResource(pong::Input{});
  ecs.addGlobalResource(pong::Renderer{});

  ecs.addGlobalResourceSystemPre(pong::Input::pollNewInputs);

  ecs.addGlobalResourceSystemPost(pong::Renderer::system);
  ecs.addGlobalResourceSystemPost(pong::Input::onFrameEnd);
  ecs.addGlobalResourceSystemPost([](ecs::Resources &r) {
    auto &dt   = r.getResource<pong::DeltaTime>()->get();
    auto &time = r.getResource<pong::Time>()->get();

    dt = pong::DeltaTime(std::min(GetFrameTime(), 1 / 60.0f));
    time += GetFrameTime();
  });

  const std::string mm = pong::sceneNames::mainMenu;
  ecs.addEmptyLevel(mm);
  ecs.addSetupSystemRes(mm, pong::setupMainMenu);
  ecs.setStartLevel(mm);

  const std::string mg = pong::sceneNames::mainGame;
  ecs.addEmptyLevel(mg);
  ecs.addSetupSystemRes(mg, pong::setupMainGame);

  const std::string go = pong::sceneNames::gameOver;
  ecs.addEmptyLevel(go);
  ecs.addSetupSystemRes(go, pong::setupGameOver);

  if (!ecs.validateLevelTransitions()) {
    std::cerr << "Invalid transitions or no start level\n";
    std::terminate();
  }

  ecs.runSetupSystems();
  while (!(WindowShouldClose() || ecs.shouldQuit())) {
    ecs.checkTransitions();

    ecs.runPreSystems();
    ecs.runPerFrameSystems();
    ecs.runPostSystems();
  }

  CloseWindow();
  return 0;
}
