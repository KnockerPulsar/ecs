#include "defs.h"
#include "ecs.h"

#include "levels/main_menu.h"

#include "raylib.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>

// TODO: fix crash when accessing currentLevel without adding levels / setting the start level
// TODO: fix crash when accessing a component type without any components (uninitialized)
int main() {

  InitWindow(800, 400, "ecs-pong");

  ecs::ECS ecs;
  ecs.addEmptyLevel("main-menu");
  ecs.addSetupSystem("main-menu", pong::setupMainMenu);
  ecs.setStartLevel("main-menu");

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
