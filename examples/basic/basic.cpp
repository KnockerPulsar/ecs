#include "defs.h"
#include "ecs.h"
#include "level.h"
#include "resources.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

struct EnemyGoal {
  u32 value;
};

struct Pos2D {
  f32 x, y;
};

struct Health {
  u32 value;
};

struct Player {
  u32 damage;
  u32 range;
};

struct Enemy {};

void checkGoalMet(ecs::ResourceBundle r, ecs::ComponentIter<Enemy, Health> enemies) {
  auto deadEnemies = 0;
  for (const auto &[_, eHealth] : enemies) {
    if (eHealth->value == 0) {
      deadEnemies += 1;
    }
  }

  // Notice that we get the resource from the level resource, where we originally put it
  auto enemyGoal = r.level.getResource<EnemyGoal>()->get();
  if (deadEnemies >= enemyGoal.value) {
    std::cout << "Enemy goal met!\n";

    // Global resource that the ECS system watches for to quit the loop
    r.global.addResource(ecs::Quit{});
  }
}

void printEnemies(ecs::ComponentIter<Enemy, Health, Pos2D> enemies) {
  for (const auto &[_, health, pos] : enemies) {
    std::cout << "Enemy at position: (" << pos->x << ',' << pos->y << "), health: " << health->value << '\n';
  }

  std::cout << '\n';
}

void damageEnemies(ecs::ComponentIter<Player, Pos2D> player, ecs::ComponentIter<Enemy, Health, Pos2D> enemies) {
  auto [playerComp, playerPos] = *player.begin();

  for (const auto &[_, eHealth, ePos] : enemies) {
    const auto dx   = playerPos->x - ePos->x;
    const auto dy   = playerPos->y - ePos->y;
    const auto dist = std::sqrt(dx * dx + dy * dy);

    if (dist < playerComp->range) {
      auto damage    = playerComp->damage / dist;
      eHealth->value = std::max<u32>(eHealth->value - damage, 0u);
    }
  }
}

void setupExampleLevel(ecs::Resources &global, ecs::Level &el) {
  // Notice that we add it to the level, not the global resources
  el.addResource(EnemyGoal{3});

  el.addEntity(Player{.damage = 23, .range = 10}, Pos2D{1, -6});

  const std::vector<Pos2D> enemyPositions = {{1, 1}, {0, 12}, {-2, 0}, {2, 2}, {0, 0}};
  for (const auto &ePos : enemyPositions) {
    el.addEntity(Enemy{}, Health{100}, Pos2D{ePos});
  }

  // Notice that that template signature and the function signature should match
  // ecs::Query<Ts...> becomes ecs::ComponentIter<Ts...>
  // Note that systems are run in the order they're added in.
  el.addPerFrameSystem<ecs::Query<Player, Pos2D>, ecs::Query<Enemy, Health, Pos2D>>(damageEnemies);
  el.addPerFrameSystem<ecs::Query<Enemy, Health, Pos2D>>(printEnemies);
  el.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Enemy, Health>>(checkGoalMet);
}

int main() {
  ecs::ECS ecs;

  ecs.addStartupLevel("exampleLevel", setupExampleLevel);
  // Add any other levels here

  // Sets up the startup level
  ecs.runSetupSystems();
  while (!ecs.shouldQuit()) {
    // ecs::TransitionToLevel messages are checked here
    ecs.checkTransitions();

    // Global systems that run before level specific per frame systems
    // Can check inputs here
    // ecs.runPreSystems();

    // Any level specific systems run here
    ecs.runPerFrameSystems();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Run after the level specific frame systems
    // Can run rendering code here
    // ecs.runPostSystems();
  }
}
