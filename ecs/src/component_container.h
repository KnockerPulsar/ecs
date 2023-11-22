#pragma once

#include "defs.h"

#include <functional>
#include <typeindex>

namespace ecs {
struct ComponentContainer {
  struct ComponentOperation {
    std::function<void()>                                     pushDummy;
    std::function<void(ComponentContainer &, Entity, Entity)> moveComponent;
    std::function<void(Entity)>                               removeComponent;
  };

  friend class ECS;
  friend struct Commands;
  std::unordered_map<std::type_index, std::any> component_vectors;

  // Need something to map a type id to a std::vector<T>::push_back
  std::unordered_map<std::type_index, ComponentOperation> componentOperations;

  uint32_t numEntities = 0;

  template <typename T>
  Vector<T> &getComponentVector() {
    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<Vector<T> &>(component_vectors[typeid(T)]);
  }

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    (lazilyInitComponentVector<Ts>(), ...);
    (lazilyRegisterComponentOperations<Ts>(), ...);

    for (auto &[type_id, any] : component_vectors) {
      componentOperations[type_id].pushDummy();
    }

    (addComponent(std::forward<Ts>(comps)), ...);

    numEntities += 1;
    return numEntities;
  }

private:
  template <typename T>
  void addComponent(T comp) {
    getComponentVector<T>()[numEntities] = comp;
  }

  template <typename T>
  void lazilyInitComponentVector() {
    auto &vec_any = component_vectors[typeid(T)];
    if (!vec_any.has_value()) {
      vec_any = std::make_any<Vector<T>>(Vector<T>(numEntities, std::nullopt));
    }
  }

  template <typename T>
  void lazilyRegisterComponentOperations() {
    if (auto f = componentOperations.find(typeid(T)); f == componentOperations.end()) {
      auto dummyPusher = [this]() { getComponentVector<T>().push_back(std::nullopt); };

      auto moveComponent = [this](ComponentContainer &oth, Entity sourceId, Entity destId) {
        auto &selfVec    = getComponentVector<T>();
        auto &otherVec   = oth.getComponentVector<T>();
        otherVec[destId] = std::move(selfVec[sourceId]);
      };

      auto removeComponent = [this](Entity eid) { getComponentVector<T>()[eid] = std::nullopt; };

      componentOperations.insert({typeid(T), ComponentOperation{dummyPusher, moveComponent, removeComponent}});
    }
  }
};
} // namespace ecs
