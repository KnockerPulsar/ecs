#include "ecs/defs.h"
#include "ecs/ecs.h"
#include "ecs/level.h"
#include "ecs/resources.h"

#include <cmath>

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};

template <typename ...Ts>
auto testArchetypes(ecs::Level& level, u32 expectedSize)
{
  level.addPerFrameSystem<ecs::Query<Ts...>>([expectedSize](ecs::ComponentIter<Ts...> comps) {
    if (auto const numberOfEntities = comps.numberOfEntities(); numberOfEntities != expectedSize) {
      std::cerr << ecs::print(ecs::Query<Ts...>{}) << " actual component count: " << numberOfEntities
                << ", expected: " << expectedSize << '\n';
      assert(false);
    }
  });
}

auto setupTestLevel(ecs::Resources & resources, ecs::Level &level) {
  auto ab1 = level.addEntity(A{}, B{}, C{});
  level.addEntity(A{}, B{});
  level.addEntity(C{}, B{}, D{});
  level.addEntity(E{});

  testArchetypes<A, B>(level, 2);
  testArchetypes<B>(level, 3);
  testArchetypes<B, C>(level, 2);
  testArchetypes<E>(level, 1);

  assert(level.archetypes.size() == 4 && "Should have 4 archetypes!");

  level.addPerFrameSystem<ecs::ResourceBundle>([&level, ab1](ecs::ResourceBundle) { level.removeEntity(ab1); });

  testArchetypes<A, B>(level, 1);
}

int main() {
  ecs::ECS ecs;

  ecs.addStartupLevel("testLevel", setupTestLevel);

  ecs.runSetupSystems();
  ecs.runPerFrameSystems();
}
