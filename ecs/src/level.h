#pragma once

// clang-format off
#include "defs.h"
#include "resources.h"
#include "component_container.h"
#include "multi_iterator.h"
#include "commands.h"
#include "tuple_utils.h"
// clang-format on

#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace ecs {

template <typename... Ts>
concept NoResources = !std::is_same_v<Nth<0, Ts...>, ResourceBundle>;

template <typename... Ts>
struct OptTupleUnwrapper;

struct Level {
  struct Transition {
    std::string           sourceLevel, destinationLevel;
    std::function<bool()> transitionCondition;
  };

  ComponentContainer components;

  std::vector<std::function<void()>>        systems;
  std::vector<std::function<void(Level &)>> setupSystems;

  Resources  levelResources;
  Resources &globalResources; // Obtained from the ECS instance containing this level.
  bool       hasBeenSetup = false;

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    components.addEntity(std::forward<Ts>(comps)...);

    return components.numEntities++;
  }

  Entity addEmptyEntity() {
    // Make space for the new entities
    for (auto &[_, op] : components.componentOperations) {
      op.pushDummy();
    }

    return components.numEntities++;
  }

  void removeEntity(Entity eid) {
    for (auto &[_, op] : components.componentOperations) {
      op.removeComponent(eid);
    }
  }

  template <typename T>
  std::optional<std::reference_wrapper<Vector<T>>> getComponentVector() {
    return components.getComponentVector<T>();
  }

  void copyComponents(Commands &cmd, Entity sourceId, Entity destId) {
    for (auto &[typeId, anyVec] : cmd.entitiesToAdd.component_vectors) {
      cmd.entitiesToAdd.componentOperations[typeId].moveComponent(components, sourceId, destId);
    }
  }

  void processCommands(Commands &cmd) {
    // Loops over all added entities, creates an empty one in the ECS storage,
    // then moves over all components from `entitiesToAdd` to the ECS storage.
    for (uint32_t sourceId = 0; sourceId < cmd.entitiesToAdd.numEntities; sourceId++) {
      auto destId = addEmptyEntity();
      copyComponents(cmd, sourceId, destId);
    }

    for (auto &&eid : cmd.entitiesToRemove) {
      removeEntity(eid);
    }
  }

  // 3 possible inputs: Global resources, level resources, components
  //
  // Components
  // Level resources
  // Global resources
  //
  // Level resources + components
  // Global resources + components
  // Global resources + level resources
  // Global resourecs + level resources + components

  // Add system with access to only to resources.
  template <typename R, typename F>
  requires(MatchSignature<F, void, R>)
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() { std::invoke(fn, {.global = globalResources, .level = levelResources}); });
  }

  // Add system WITHOUT access to resources.
  template <typename... Query, typename F>
  requires(NoResources<Query...>)
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto queryIters = std::make_tuple(components.getQueryIter(Query{})...);

      if (allSome(queryIters)) {
        const auto unwrappedIters = unwrapTuple(queryIters);
        std::apply(fn, unwrappedIters);
      }
    });
  }

  // Add system with access to level resources + components.
  template <typename R, typename... Query, typename F>
  requires(std::is_same_v<ResourceBundle, R> && sizeof...(Query) > 0)
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto queryIters = std::make_tuple(components.getQueryIter(Query{})...);

      if (allSome(queryIters)) {
        const auto unwrappedIters = unwrapTuple(queryIters);
        auto       rb             = ResourceBundle{.global = globalResources, .level = levelResources};
        std::apply(fn, std::tuple_cat(std::make_tuple(rb), unwrappedIters));
      }
    });
  }

  void runSystems() {
    for (auto &sys : systems) {
      sys();
    }

    if (auto cmd = levelResources.getResource<Commands>()) {
      processCommands(*cmd);
    }
  }

  template <typename F>
  void addSetupSystem(F &&fn) {
    setupSystems.push_back(fn);
  }

  void runSetupSystems() {
    for (auto &setupSys : setupSystems) {
      setupSys(*this);
    }
  }

  template <typename R>
  void addResource(R initialValue) {
    levelResources.addResource(initialValue);
  }
};
} // namespace ecs
