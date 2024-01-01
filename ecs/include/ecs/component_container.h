#pragma once

#include "defs.h"

#include <any>
#include <functional>
#include <iostream>
#include <optional>
#include <typeindex>

namespace ecs {

// Dense component storage.
// Each extra component adds an extra vector with space for all existing entities.
struct ComponentContainer {

  // Hold compile time information about the type of the component.
  struct ComponentOperation {
    std::function<void()>                                     pushDummy;
    std::function<void(ComponentContainer &, Entity, Entity)> moveComponent;
    std::function<void(Entity)>                               removeComponent;
  };

  friend class ECS;
  friend struct Commands;

  // Contain the data for all entities
  std::unordered_map<std::type_index, std::any> componentVectors;

  // Need something to map a type id to a std::vector<T>::push_back
  std::unordered_map<std::type_index, ComponentOperation> componentOperations;

  uint32_t numEntities = 0;

  // Given a type, get an optional (might not exist) reference to its component vector.
  template <typename T>
  std::optional<std::reference_wrapper<Vector<T>>> getComponentVector() {
    if (!componentVectors.contains(typeid(T))) {
      return {};
    }

    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<Vector<T> &>(componentVectors.at(typeid(T)));
  }

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    (lazilyInitComponentVector<Ts>(), ...);
    (lazilyRegisterComponentOperations<Ts>(), ...);

    for (auto &[type_id, op] : componentOperations) {
      op.pushDummy();
    }

    (addComponent(std::forward<Ts>(comps)), ...);

    return numEntities++;
  }

  // Optional over the OptIters since the component vector might not exist.
  template <typename... Ts>
  using EntityQueryResult = std::tuple<std::optional<OptIter<Ts>>...>;

  template <typename T>
  std::optional<OptIter<T>> getEntityComponentIter(Entity eid) {
    auto compVec = getComponentVector<T>();

    if (!compVec.has_value()) {
      return {};
    }

    return compVec->get().begin() + eid;
  }

  template <typename... Ts>
  EntityQueryResult<Ts...> getEntityComponents(uint entityId) {
    return std::make_tuple(getEntityComponentIter<Ts>(entityId)...);
  }

  template <typename... Ts>
  bool allComponentsExist() {
    return (getComponentVector<Ts>().has_value() && ...);
  }

  template <typename... Ts>
  requires(sizeof...(Ts) > 0)
  std::optional<MultiIterator<Ts...>> getQueryIter(Query<Ts...> && /*DUMMY*/) {
    if (!allComponentsExist<Ts...>()) {
      std::cerr << "Attempt to run a system with no available components (" << typeid(std::tuple<Ts...>).name()
                << ").\n";
      return std::nullopt;
    }

    return MultiIterator<Ts...>(*this);
  }

  void clear() {
    componentVectors.clear();
    componentOperations.clear();
    numEntities = 0;
  }

private:
  template <typename T>
  void addComponent(T comp) {
    auto compVec                = getComponentVector<T>();
    compVec->get()[numEntities] = comp;
  }

  template <typename T>
  void lazilyInitComponentVector() {
    if (!componentVectors.contains(typeid(T))) {
      componentVectors.insert({typeid(T), std::make_any<Vector<T>>(numEntities)});
    }
  }

  template <typename T>
  void lazilyRegisterComponentOperations() {
    if (auto f = componentOperations.find(typeid(T)); f == componentOperations.end()) {
      auto dummyPusher = [this]() { getComponentVector<T>()->get().push_back(std::nullopt); };

      auto moveComponent = [this](ComponentContainer &oth, Entity sourceId, Entity destId) {
        auto selfVec  = getComponentVector<T>();
        auto otherVec = oth.getComponentVector<T>();

        // FIXME: Need to check if either vector doesn't exist
        otherVec.value().get()[destId] = std::move(selfVec.value().get()[sourceId]);
      };

      auto removeComponent = [this](Entity eid) {
        auto compVec = getComponentVector<T>();
        if (compVec) {
          compVec.value().get()[eid] = std::nullopt;
        } else {
          std::cerr << "Attempt to remove a component that doesn't exist.\n";
        }
      };

      componentOperations.insert({typeid(T), ComponentOperation{dummyPusher, moveComponent, removeComponent}});
    }
  }
};

} // namespace ecs
