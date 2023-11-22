#pragma once

// clang-format off
#include "defs.h"
#include "component_container.h"
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
struct Commands {
  ComponentContainer  entitiesToAdd;
  std::vector<Entity> entitiesToRemove;

  template <typename... Ts>
  void addEntity(Ts &&...comps) {
    entitiesToAdd.addEntity(std::forward<Ts>(comps)...);
  }

  void removeEntity(Entity e) { entitiesToRemove.push_back(e); }
};

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
  template <typename T>
  struct Resource {
    T r;
  };

  struct Transition {
    std::string           sourceLevel, destinationLevel;
    std::function<bool()> transitionCondition;
  };

  template <typename... Ts>
  struct MultiIterator {

    struct iterator {
      std::tuple<OptIter<Ts>...> current;
      std::tuple<Iter<Ts>...>    currentForInc;

      iterator(std::tuple<OptIter<Ts>...> c) : current(c) {}

      bool allSome() {
        return std::apply([](auto &&...args) { return (args->has_value() && ...); }, current);
      }

      template <unsigned int N = sizeof...(Ts) - 1>
      static void advance_all(std::tuple<OptIter<Ts>...> &iters) {
        std::get<N>(iters)++;

        if constexpr (N >= 1) {
          advance_all<N - 1>(iters);
        }
      }

      template <unsigned int N = 0, unsigned int NEnd = sizeof...(Ts) - 1>
      static auto unwrapIterators(std::tuple<OptIter<Ts>...> iterTuple) {
        using T = typename std::tuple_element<N, std::tuple<Ts...>>::type;
        Iter<T>    comp_it(&(std::get<N>(iterTuple)->value()));
        const auto t = std::make_tuple(comp_it);

        if constexpr (N == NEnd) {
          return t;
        } else {
          return std::tuple_cat(t, unwrapIterators<N + 1>(iterTuple));
        }
      }

      std::tuple<OptIter<Ts>...> &optDeref() { return current; }
      std::tuple<Iter<Ts>...>    &operator*() {
           currentForInc = unwrapIterators(current);
           return currentForInc;
      }
      std::tuple<Iter<Ts>...> *operator->() { return &unwrapIterators(current); }

      iterator &operator++() {
        advance_all(current);
        return *this;
      }

      iterator operator++(int) {
        auto temp = *this;
        advance_all(current);
        return temp;
      }

      friend bool operator==(const iterator &a, const iterator &b) { return a.current == b.current; }
    };

    Level   &owner;
    iterator _begin, _end;

    MultiIterator(Level &o) : owner(o), _begin(getBegins(std::tuple<Ts...>{})), _end(getEnds(std::tuple<Ts...>{})) {}

    template <typename T2, typename... Ts2>
    auto getBegins(std::tuple<T2, Ts2...> /*UNUSED*/) {
      auto t_begin = std::make_tuple(owner.getComponentVector<T2>().begin());

      if constexpr (sizeof...(Ts2) > 0) {
        auto remaining = getBegins(std::tuple<Ts2...>{});
        return std::tuple_cat(t_begin, remaining);
      } else
        return t_begin;
    }

    template <typename T2, typename... Ts2>
    auto getEnds(std::tuple<T2, Ts2...> /*UNUSED*/) {
      auto t_end = std::make_tuple(owner.getComponentVector<T2>().end());

      if constexpr (sizeof...(Ts2) > 0) {
        auto remaining = getEnds(std::tuple<Ts2...>{});
        return std::tuple_cat(t_end, remaining);
      } else
        return t_end;
    }

    iterator begin() { return _begin; }
    iterator end() { return _end; }

    template <typename F>
    void forEach(F &&fn) {
      for (iterator iter = _begin; iter != _end; iter++) {
        if (!iter.allSome())
          continue;
        std::apply(fn, *iter);
      }
    }

    template <typename F>
    void forEachMut(F &&fn, Commands &cmd) {
      for (iterator iter = _begin; iter != _end; iter++) {
        if (!iter.allSome())
          continue;
        auto paramTuple = std::tuple_cat(std::make_tuple(std::ref(cmd)), *iter);
        std::apply(fn, paramTuple);
      }
    }

    template <typename F>
    void forEachMutId(F &&fn, Commands &cmd) {
      auto eid = 0;
      for (iterator iter = _begin; iter != _end; iter++) {
        if (!iter.allSome())
          continue;

        auto paramTuple = std::tuple_cat(std::make_tuple(eid), std::make_tuple(std::ref(cmd)), *iter);
        std::apply(fn, paramTuple);
      }
    }
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

  template <typename... Ts, typename F>
  void forEach(F &&fn) {
    MultiIterator<Ts...>(*this).forEach(fn);
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

  template <typename... Ts, typename F>
  void forEachMut(F &&fn) {
    Commands cmd{};
    MultiIterator<Ts...>(*this).forEachMut(fn, cmd);

    processCommands(cmd);
  }

  template <typename... Ts, typename F>
  void forEachMutId(F &&fn) {
    Commands cmd{};
    MultiIterator<Ts...>(*this).forEachMutId(fn, cmd);

    processCommands(cmd);
  }

  // Add system with access to resources.
  // checks if the first template parameter is of type `ecs::Resources`.
  template <typename R, typename... Ts, typename F>
  requires(std::is_same_v<R, ecs::Resources>) void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto multiIter = MultiIterator<Ts...>{*this};
      std::invoke(fn, std::ref(resources), multiIter);
    });
  }

  // Add system WITHOUT access to resources.
  // Checks if the first parameter pack type is not `ecs::Resources`.
  template <typename... Ts, typename F>
  requires(FirstNotResource<Ts...>) void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto multiIter = MultiIterator<Ts...>{*this};
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

} // namespace ecs

void demo() {
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

  level.forEach<Foo, Baz>([](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });

  std::cout << "===\n";

  level.forEachMut<Foo, Baz>([](ecs::Commands &cmd, auto f, auto b) {
    if (f->x > 100) {
      cmd.addEntity(Foo{0}, Baz{100.0});
    }
  });

  level.forEach<Foo, Baz>([](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });

  std::cout << "===\n";

  level.forEachMutId<Foo, Baz>([](ecs::Entity selfId, ecs::Commands &cmd, auto f, auto b) {
    if (b->z == 100) {
      cmd.removeEntity(selfId);
    }
  });

  level.forEach<Foo, Baz>([](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });
}
