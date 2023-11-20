#include <algorithm>
#include <any>
#include <cstdint>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
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

template <typename T> using Vector = typename std::vector<std::optional<T>>;
template <typename T> using OptIter = typename Vector<T>::iterator;
template <typename T> using Iter = typename std::vector<T>::iterator;

class ECS {

  std::unordered_map<std::type_index, std::any> component_vectors;

  std::unordered_map<std::type_index, std::function<void(std::any &)>>
      dummyPushers; // Need something to map a type id to a
                    // std::vector<T>::push_back

  uint32_t numEntities = 0;

  template <typename... Ts> struct MultiIterator {
    ECS &owner;
    std::tuple<OptIter<Ts>...> begins, ends;

    MultiIterator(ECS &o)
        : owner(o), begins(getBegins(std::tuple<Ts...>{})),
          ends(getEnds(std::tuple<Ts...>{})) {}

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
      Iter<T> comp_it(&(std::get<N>(iterTuple)->value()));
      const auto t = std::make_tuple(comp_it);

      if constexpr (N == NEnd) {
        return t;
      } else {
        return std::tuple_cat(t, unwrapIterators<N + 1>(iterTuple));
      }
    }

    template <typename F> void forEach(F &&fn) {

      for (auto current = begins; current != ends;
           advance_all<sizeof...(Ts) - 1>(current)) {
        const auto all_some = std::apply(
            [](auto &&...args) { return (args->has_value() && ...); }, current);

        if (all_some) {
          std::apply(fn, unwrapIterators(current));
        }
      }
    }

    template <unsigned int N, typename... Ts2>
    void advance_all(std::tuple<Ts2...> &iters) {
      std::get<N>(iters)++;

      if constexpr (N >= 1) {
        advance_all<N - 1, Ts2...>(iters);
      }
    }
  };

public:
  template <typename... Ts> Entity addEntity(Ts &&...comps) {
    (lazilyInitComponentVector<Ts>(), ...);
    (lazilyRegisterDummyPusher<Ts>(), ...);

    for (auto &[type_id, any] : component_vectors) {
      dummyPushers[type_id](any);
    }

    (addComponent(std::forward<Ts>(comps)), ...);

    numEntities += 1;
    return numEntities - 1;
  }

  template <typename T> auto &getComponentVector() {
    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<Vector<T> &>(component_vectors[typeid(T)]);
  }

  template <typename T> void lazilyInitComponentVector() {
    auto &vec_any = component_vectors[typeid(T)];
    if (!vec_any.has_value()) {
      vec_any = std::make_any<Vector<T>>(Vector<T>(numEntities, std::nullopt));
    }
  }

  template <typename T> void lazilyRegisterDummyPusher() {
    if (auto f = dummyPushers.find(typeid(T)); f == dummyPushers.end()) {
      auto dummyPusher = [this](std::any &av) {
        getComponentVector<T>().push_back(std::nullopt);
      };

      dummyPushers.insert({typeid(T), dummyPusher});
    }
  }

  template <typename T> void addComponent(T comp) {
    getComponentVector<T>()[numEntities] = comp;
  }

  template <typename... Ts, typename F> void forEach(F &&fn) {
    MultiIterator<Ts...>(*this).forEach(fn);
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

  ecs.forEach<Foo, Baz>(
      [](auto f, auto b) { std::cout << f->x << " " << b->z << '\n'; });

  return 0;
}
