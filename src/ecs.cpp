#include <algorithm>
#include <any>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

struct Foo {
  int x = 42;
};

struct Bar {
  std::string y = "bar";
};

struct Baz {
  float z = 3.14;
};

namespace ecs {
using Entity = std::uint32_t;

template <typename T>
using Vector = typename std::vector<std::optional<T>>;
template <typename T>
using OptIter = typename Vector<T>::iterator;
template <typename T>
using Iter = typename std::vector<T>::iterator;

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

struct Commands {
  ComponentContainer  entitiesToAdd;
  std::vector<Entity> entitiesToRemove;

  template <typename... Ts>
  void addEntity(Ts &&...comps) {
    entitiesToAdd.addEntity(std::forward<Ts>(comps)...);
  }

  void removeEntity(Entity e) { entitiesToRemove.push_back(e); }
};

class ECS {
  template <typename... Ts>
  struct MultiIterator {
    ECS                       &owner;
    std::tuple<OptIter<Ts>...> begins, ends;

    MultiIterator(ECS &o) : owner(o), begins(getBegins(std::tuple<Ts...>{})), ends(getEnds(std::tuple<Ts...>{})) {}

    template <typename T2, typename... Ts2>
    auto getBegins(std::tuple<T2, Ts2...>) {
      auto t_begin = std::make_tuple(owner.getComponentVector<T2>().begin());

      if constexpr (sizeof...(Ts2) > 0) {
        auto remaining = getBegins(std::tuple<Ts2...>{});
        return std::tuple_cat(t_begin, remaining);
      } else
        return t_begin;
    }

    template <typename T2, typename... Ts2>
    auto getEnds(std::tuple<T2, Ts2...>) {
      auto t_end = std::make_tuple(owner.getComponentVector<T2>().end());

      if constexpr (sizeof...(Ts2) > 0) {
        auto remaining = getEnds(std::tuple<Ts2...>{});
        return std::tuple_cat(t_end, remaining);
      } else
        return t_end;
    }

    template <unsigned int N = 0, unsigned int NEnd = sizeof...(Ts) - 1>
    auto unwrapIterators(std::tuple<OptIter<Ts>...> iterTuple) {
      using T = typename std::tuple_element<N, std::tuple<Ts...>>::type;
      Iter<T>    comp_it(&(std::get<N>(iterTuple)->value()));
      const auto t = std::make_tuple(comp_it);

      if constexpr (N == NEnd) {
        return t;
      } else {
        return std::tuple_cat(t, unwrapIterators<N + 1>(iterTuple));
      }
    }

    template <unsigned int N, typename... Ts2>
    void advance_all(std::tuple<Ts2...> &iters) {
      std::get<N>(iters)++;

      if constexpr (N >= 1) {
        advance_all<N - 1, Ts2...>(iters);
      }
    }

    template <typename F>
    void forEach(F &&fn) {

      for (auto current = begins; current != ends; advance_all<sizeof...(Ts) - 1>(current)) {
        const auto all_some = std::apply([](auto &&...args) { return (args->has_value() && ...); }, current);

        if (all_some) {
          std::apply(fn, unwrapIterators(current));
        }
      }
    }

    template <typename F>
    void forEachMut(F &&fn, Commands &cmd) {

      for (auto current = begins; current != ends; advance_all<sizeof...(Ts) - 1>(current)) {

        const auto all_some = std::apply([](auto &&...args) { return (args->has_value() && ...); }, current);

        if (all_some) {
          auto paramTuple = std::tuple_cat(std::make_tuple(std::ref(cmd)), unwrapIterators(current));
          std::apply(fn, paramTuple);
        }
      }
    }

    template <typename F>
    void forEachMutId(F &&fn, Commands &cmd) {

      auto multiIter = begins;
      auto eid       = 0;
      for (; multiIter != ends; advance_all<sizeof...(Ts) - 1>(multiIter), eid++) {

        const auto all_some = std::apply([](auto &&...args) { return (args->has_value() && ...); }, multiIter);

        if (all_some) {
          auto paramTuple =
              std::tuple_cat(std::make_tuple(eid), std::make_tuple(std::ref(cmd)), unwrapIterators(multiIter));
          std::apply(fn, paramTuple);
        }
      }
    }
  };

public:
  ComponentContainer components;

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
};

} // namespace ecs

int main() {
  ecs::ECS ecs;

  const auto t1 = ecs.addEntity(Foo{}, Bar{});
  const auto t2 = ecs.addEntity(Foo{69}, Bar{"urmom"});
  const auto t3 = ecs.addEntity(Baz{});
  const auto t4 = ecs.addEntity(Baz{12.14});
  const auto t5 = ecs.addEntity(Foo{122}, Baz{12.14});

  ecs.forEach<Foo, Baz>([](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });

  std::cout << "===\n";

  ecs.forEachMut<Foo, Baz>([](ecs::Commands &cmd, auto f, auto b) {
    if (f->x > 100) {
      cmd.addEntity(Foo{0}, Baz{100.0});
    }
  });

  ecs.forEach<Foo, Baz>([](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });

  std::cout << "===\n";

  ecs.forEachMutId<Foo, Baz>([](ecs::Entity selfId, ecs::Commands &cmd, auto f, auto b) {
    if (b->z == 100) {
      cmd.removeEntity(selfId);
    }
  });

  ecs.forEach<Foo, Baz>([](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });
  return 0;
}
