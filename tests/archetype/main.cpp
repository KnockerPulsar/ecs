#include "defs.h"
#include "ecs.h"
#include "level.h"

#include <cmath>

struct A {};
struct B {};
struct C {};
struct D {};
struct E {};

template <typename ...Ts>
auto testArchetypes(ecs::Level& level, int expectedSize)
{
  auto const abCount   = 2;
  level.addPerFrameSystem<ecs::Query<Ts...>>([expectedSize](ecs::ComponentIter<Ts...> comps) {
    auto counter = 0;
    for (auto const _ : comps)
      counter++;

    if (counter != expectedSize) {
      std::cerr << ecs::print(ecs::Query<Ts...>{}) << " actual component count: " << counter
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

  level.removeEntity(ab1);
  testArchetypes<A, B>(level, 1);

  resources.addResource(ecs::Quit{});
}

int main() {
  ecs::ECS ecs;

  ecs.addStartupLevel("testLevel", setupTestLevel);

  ecs.runSetupSystems();
  ecs.runPerFrameSystems();
}
