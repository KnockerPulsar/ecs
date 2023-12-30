#pragma once

#include "defs.h"
#include "tuple_utils.h"

#include <any>
#include <functional>
#include <iostream>
#include <optional>
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

  std::unordered_map<std::type_index, std::any> componentVectors;

  // Need something to map a type id to a std::vector<T>::push_back
  std::unordered_map<std::type_index, ComponentOperation> componentOperations;

  uint32_t numEntities = 0;

  template <typename T>
  std::optional<std::reference_wrapper<Vector<T>>> getComponentVector() {
    if (!componentVectors.contains(typeid(T))) {
      return {};
    }

    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<Vector<T> &>(componentVectors[typeid(T)]);
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
  template <typename T2, typename... Ts2>
  using EntityQueryResult = std::tuple<std::optional<OptIter<T2>>, std::optional<OptIter<Ts2>>...>;

  template <typename T2, typename... Ts2>
  EntityQueryResult<T2, Ts2...> getBegins() {
    return getEntityComponents<T2, Ts2...>(0);
  }

  template <typename T2, typename... Ts2>
  EntityQueryResult<T2, Ts2...> getEnds() {
    return getEntityComponents<T2, Ts2...>(numEntities - 1);
  }

  template <typename T2, typename... Ts2>
  EntityQueryResult<T2, Ts2...> getEntityComponents(uint entityId) {
    auto compVec = getComponentVector<T2>();

    std::tuple<std::optional<ecs::OptIter<T2>>> t_begin =
        std::make_tuple(compVec.has_value() ? (compVec->get().begin() + entityId) : std::optional<ecs::OptIter<T2>>());

    if constexpr (sizeof...(Ts2) > 0) {
      auto remaining = getEntityComponents<Ts2...>(entityId);
      return std::tuple_cat(t_begin, remaining);
    } else {
      return t_begin;
    }
  }

  template <typename... Ts>
  bool allComponentsExist() {
    return allSome(getBegins<Ts...>());
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
