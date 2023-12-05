#include "defs.h"
#include "ecs.h"

#include "level.h"
#include "levels/main_menu.h"
#include "levels/main_game.h"

#include "raylib.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {

  InitWindow(800, 400, "ecs-pong");

  ecs::ECS ecs;

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
	return l.resources.consumeResource<pong::PlayChosen>().has_value();
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
    ecs.runSystems();
  }

  CloseWindow();
  return 0;
}
