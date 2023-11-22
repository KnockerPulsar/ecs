#pragma once
#include "./component_container.h"

namespace ecs {
struct Commands {
  ComponentContainer  entitiesToAdd;
  std::vector<Entity> entitiesToRemove;

  template <typename... Ts>
  void addEntity(Ts &&...comps) {
    entitiesToAdd.addEntity(std::forward<Ts>(comps)...);
  }

  void removeEntity(Entity e) { entitiesToRemove.push_back(e); }
};
} // namespace ecs
