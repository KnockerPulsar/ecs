#include "defs.h"
#include "ecs.h"

#include "level.h"
#include "levels/main_menu.h"
#include "levels/main_game.h"
#include "levels/common.h"

#include "raylib.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {

  InitWindow(800, 400, "ecs-pong");

  ecs::ECS ecs;

  ecs.addGlobalResource(pong::Input{});
  ecs.addGlobalResource(pong::Time{0.0});
  ecs.addGlobalResource(pong::Renderer{});

  ecs.addGlobalResourceSystemPre(pong::Input::pollNewInputs);

  ecs.addGlobalResourceSystemPost(pong::Renderer::system);
  ecs.addGlobalResourceSystemPost([](ecs::GlobalResources& r) {
      auto &time = r.getResource<pong::Time>().value().get();
      time += GetFrameTime();
  });
  ecs.addGlobalResourceSystemPost(pong::Input::onFrameEnd);

  ecs.addEmptyLevel("main-menu");
  ecs.addSetupSystem("main-menu", pong::setupMainMenu);
  ecs.setStartLevel("main-menu");

  ecs.addEmptyLevel("main-game");
  ecs.addSetupSystem("main-game", pong::setupMainGame);

  ecs.addTransition(ecs::Level::Transition{
      .sourceLevel = "main-menu",
      .destinationLevel = "main-game",
      .transitionCondition = [&]() {
	auto l = ecs.getLevel("main-menu").value().get();
	return l.globalResources.consumeResource<pong::PlayChosen>().has_value();
      }
  });

  {
    if (!ecs.validateLevelTransitions()) {
      std::cerr << "Invalid transitions or no start level\n";
      std::terminate();
    }
  }

  ecs.runSetupSystems();
  while (!WindowShouldClose()) {
    ecs.checkTransitions();

    ecs.runPreSystems();
    ecs.runSystems();
    ecs.runPostSystems();
  }

  CloseWindow();
  return 0;
}
