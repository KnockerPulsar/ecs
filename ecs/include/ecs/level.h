#pragma once

// clang-format off
#include "defs.h"
#include "resources.h"
#include "component_container.h"
#include "commands.h"
#include "tuple_utils.h"
// clang-format on

#include <functional>
#include <iostream>
#include <optional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace ecs {

template <typename... Ts>
concept NoResources = !std::is_same_v<Nth<0, Ts...>, ResourceBundle>;

template <typename... Ts>
struct OptTupleUnwrapper;

struct Level {
  friend class ECS;

  ComponentContainer components;

  // Run every frame
  std::vector<std::function<void()>> perFrameSystems;

  // Systems that run when we transition to another level
  // Meant to reset everything to some initial state.
  std::vector<std::function<void()>> resetSystems;

  // Run whenever the system is fresh (first time, or after a reset)
  // Add entities, systems, and level/global resources.
  std::vector<std::function<void(Resources &, Level &)>> setupSystems;

  Resources  levelResources;
  Resources &globalResources; // Obtained from the ECS instance containing this level.
  bool       hasBeenSetup = false;

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    return components.addEntity(std::forward<Ts>(comps)...);
  }

  // 2 possible inputs: resources, queries
  //
  // resources only
  // queries only
  // resources + queries

  // Add system with access to resources only.
  template <typename R, typename F>
  requires(MatchSignature<F, void, R>)
  void addPerFrameSystem(F &&fn) {
    addSystemResOnly<R>(perFrameSystems, fn);
  }

  // Add system with access to components only.
  template <typename... Query, typename F>
  requires(NoResources<Query...>)
  void addPerFrameSystem(F &&fn) {
    addSystemQueryOnly<Query...>(perFrameSystems, fn);
  }

  // Add system with access to level resources + components.
  template <typename R, typename... Query, typename F>
  requires(std::is_same_v<ResourceBundle, R> && sizeof...(Query) > 0)
  void addPerFrameSystem(F &&fn) {
    addSystem<R, Query...>(perFrameSystems, fn);
  }

  template <typename R, typename F>
  requires(MatchSignature<F, void, R>)
  void addResetSystem(F &&fn) {
    addSystemResOnly<R>(resetSystems, fn);
  }

  template <typename... Query, typename F>
  requires(NoResources<Query...>)
  void addResetSystem(F &&fn) {
    addSystemQueryOnly<Query...>(resetSystems, fn);
  }

  template <typename R, typename... Query, typename F>
  requires(std::is_same_v<ResourceBundle, R> && sizeof...(Query) > 0)
  void addResetSystem(F &&fn) {
    addSystem<R, Query...>(resetSystems, fn);
  }

  template <typename R>
  void addResource(R initialValue) {
    levelResources.addResource(initialValue);
  }

  template <typename F>
  requires(MatchSignature<F, void, ecs::Resources &, ecs::Level &>)
  void addSetupSystem(F &&fn) {
    setupSystems.push_back(fn);
  }

  // Wipe the level clean, allowing setup systems to be run again;
  void completeReset() {
    components.clear();
    perFrameSystems.clear();
    resetSystems.clear();
    levelResources.clear();
    hasBeenSetup = false;
  }

private:
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

  void copyComponents(Commands &cmd, Entity sourceId, Entity destId) {
    for (auto &[typeId, anyVec] : cmd.entitiesToAdd.componentVectors) {
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

  // Add system with access to only to resources.
  template <typename R, typename F>
  void addSystemResOnly(std::vector<std::function<void()>> &systemCollection, F &&fn) {
    systemCollection.push_back([this, fn]() {
      std::invoke(fn, ResourceBundle{.global = globalResources, .level = levelResources});
    });
  }

  // Add system WITHOUT access to resources.
  template <typename... Query, typename F>
  void addSystemQueryOnly(std::vector<std::function<void()>> &systemCollection, F &&fn) {
    systemCollection.push_back([this, fn]() {
      auto queryIters = std::make_tuple(components.getQueryIter(Query{})...);

      if (allSome(queryIters)) {
        const auto unwrappedIters = unwrapTuple(queryIters);
        std::apply(fn, unwrappedIters);
      }
    });
  }

  // Add system with access to level resources + components.
  //
  // The last part of the requires clause is to get around some weird quirk in the compile where it
  // sees that Query = {void(ecs::ResourceBundle)} and F = void(ecs::ResourceBundle) for some reason.
  template <typename R, typename... Query, typename F>
  void addSystem(std::vector<std::function<void()>> &systemCollection, F &&fn) {
    systemCollection.push_back([this, fn]() {
      auto queryIters = std::make_tuple(components.getQueryIter(Query{})...);

      if (allSome(queryIters)) {
        const auto unwrappedIters = unwrapTuple(queryIters);
        auto       rb             = ResourceBundle{.global = globalResources, .level = levelResources};
        std::apply(fn, std::tuple_cat(std::make_tuple(rb), unwrappedIters));
      }
    });
  }

  void runSetupSystems() {
    if (hasBeenSetup) {
      std::cout << "Attempt to setup a level that's already been setup\n";
      return;
    }

    for (auto &setupSys : setupSystems) {
      setupSys(globalResources, *this);
    }

    hasBeenSetup = true;
  }

  void runPerFrameSystems() {
    for (auto &sys : perFrameSystems) {
      sys();
    }

    if (auto cmd = levelResources.getResource<Commands>()) {
      processCommands(*cmd);
    }
  }

  void runResetSystems() {
    for (auto &cleanupSys : resetSystems) {
      cleanupSys();
    }
  }
};
} // namespace ecs
