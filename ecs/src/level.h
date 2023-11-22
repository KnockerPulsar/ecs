#pragma once

// clang-format off
#include "defs.h"
#include "component_container.h"
#include "multi_iterator.h"
#include "commands.h"
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
struct Resources {
  std::unordered_map<std::type_index, std::any> r;

  template <typename R>
  void addResource(R initialValue) {
    r.insert({typeid(R), initialValue});
  }

  template <typename R>
  std::optional<std::reference_wrapper<R>> getResource() {
    if (!r.contains(typeid(R)))
      return std::nullopt;

    return std::any_cast<R &>(r.find(typeid(R))->second);
  }
};

template <typename... Ts>
concept FirstNotResource = !std::is_same_v<ecs::Resources, std::tuple_element_t<0, std::tuple<Ts...>>>;

struct Level {
  struct Transition {
    std::string           sourceLevel, destinationLevel;
    std::function<bool()> transitionCondition;
  };

  ComponentContainer                        components;
  std::vector<std::function<void()>>        systems;
  std::vector<std::function<void(Level &)>> setupSystems;
  Resources                                 resources;

  Entity addEmptyEntity() {
    // Make space for the new entities
    for (auto &[typeId, any_vec] : components.component_vectors) {
      components.componentOperations[typeId].pushDummy();
    }

    return components.numEntities;
  }

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    components.addEntity(std::forward<Ts>(comps)...);

    return components.numEntities;
  }

  void removeEntity(Entity eid) {
    for (auto &[_, op] : components.componentOperations) {
      op.removeComponent(eid);
    }
  }

  template <typename T>
  auto &getComponentVector() {
    return components.getComponentVector<T>();
  }

  template <typename T2, typename... Ts2>
  auto getBegins() {
    auto t_begin = std::make_tuple(getComponentVector<T2>().begin());

    if constexpr (sizeof...(Ts2) > 0) {
      auto remaining = getBegins<Ts2...>();
      return std::tuple_cat(t_begin, remaining);
    } else
      return t_begin;
  }

  template <typename T2, typename... Ts2>
  auto getEnds() {
    auto t_end = std::make_tuple(getComponentVector<T2>().end());

    if constexpr (sizeof...(Ts2) > 0) {
      auto remaining = getEnds<Ts2...>();
      return std::tuple_cat(t_end, remaining);
    } else
      return t_end;
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

  // Add system with access to resources.
  // checks if the first template parameter is of type `ecs::Resources`.
  template <typename R, typename... Ts, typename F>
  requires(std::is_same_v<R, ecs::Resources>) void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto multiIter = MultiIterator<Ts...>{getBegins<Ts...>(), getEnds<Ts...>()};
      std::invoke(fn, std::ref(resources), multiIter);
    });
  }

  // Add system WITHOUT access to resources.
  // Checks if the first parameter pack type is not `ecs::Resources`.
  template <typename... Ts, typename F>
  requires(FirstNotResource<Ts...>) void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto multiIter = MultiIterator<Ts...>{getBegins<Ts...>(), getEnds<Ts...>()};
      std::invoke(fn, multiIter);
    });
  }

  template <typename F>
  void addSetupSystem(F &&fn) {
    setupSystems.push_back(fn);
  }

  void runSystems() {
    for (auto &sys : systems) {
      sys();
    }

    if (auto cmd = resources.getResource<Commands>()) {
      processCommands(*cmd);
    }
  }

  void runSetupSystems() {
    for (auto &setupSys : setupSystems) {
      setupSys(*this);
    }
  }

  template <typename R>
  void addResource(R initialValue) {
    resources.addResource(initialValue);
  }
};

template <typename... Ts>
using ComponentIter = MultiIterator<Ts...>;

void levelDemo() {
  ecs::Level level;

  struct Foo {
    int x = 42;
  };

  struct Bar {
    std::string y = "bar";
  };

  struct Baz {
    float z = 3.14;
  };

  level.addEntity(Foo{}, Bar{});
  level.addEntity(Foo{69}, Bar{"urmom"});
  level.addEntity(Baz{});
  level.addEntity(Baz{12.14});
  level.addEntity(Foo{122}, Baz{12.14});

  auto printFooBaz = [](ecs::ComponentIter<Foo, Baz> it) {
    for (auto &[f, b] : it) {
      std::cout << f->x << " " << b->z << '\n';
    }

    std::cout << "===\n";
  };

  auto addFoo = [](ecs::Resources &r, ecs::ComponentIter<Foo, Baz> it) {
    auto &cmd = r.getResource<ecs::Commands>().value().get();
    for (auto &[f, b] : it) {
      // NOTE: Commands are processed after all systems run, thus we need to run this level at least twice to get the
      // print to display.
      if (f->x > 100) {
        cmd.addEntity(Foo{0}, Baz{100.0});
      }
    }
  };

  // TODO: Add a version of `addSystem` for Ids
  /* auto removeFoo = [](ecs::Resources &r, ecs::Iter<Foo> f, ecs::Iter<Baz> b) { */
  /*   auto &cmd = r.getResource<ecs::Commands>().value().get(); */
  /*   if (b->z == 100) { */
  /*     cmd.removeEntity(selfId); */
  /*   } */
  /* }; */

  level.addResource(ecs::Commands{});
  level.addSystem<Foo, Baz>(printFooBaz);
  level.addSystem<ecs::Resources, Foo, Baz>(addFoo);
  /* level.addSystem<ecs::Resources, Foo, Baz>(removeFoo); */
  /* level.addSystem<Foo, Baz>(printFooBaz); */

  for (int i = 0; i < 2; i++) {
    level.runSystems();
  }
}
} // namespace ecs
