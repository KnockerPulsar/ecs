#include <algorithm>
#include <any>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ecs {
using Entity = std::uint32_t;

class ECS {
  std::unordered_map<std::type_index, std::any> component_vectors;

public:
  template <typename... Ts> Entity addEntity(Ts &&...comps) {
    (addComponent(std::forward<Ts>(comps)), ...);

    return component_vectors.size();
  }

  template <typename T> auto &getComponentVector() {
    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<std::vector<T> &>(component_vectors[typeid(T)]);
  }

  template <typename T> void addComponent(T comp) {
    auto &vec_any = component_vectors[typeid(T)];
    if (vec_any.has_value()) {
      getComponentVector<T>().push_back(comp);
    } else {
      vec_any = std::make_any<std::vector<T>>({comp});
    }
  }

  template <typename T> typename std::vector<T>::iterator components_begin() {
    return getComponentVector<T>().begin();
  }

  template <typename T> typename std::vector<T>::iterator components_end() {
    return getComponentVector<T>().end();
  }

  template <typename T, typename... Ts> auto getBegins(std::tuple<T, Ts...>) {
    if constexpr (sizeof...(Ts) > 0) {
      auto remaining = getBegins(std::tuple<Ts...>{});
      return std::tuple_cat(std::make_tuple(components_begin<T>()), remaining);
    } else
      return std::make_tuple(components_begin<T>());
  }

  // Wrapper function to initialize the recursive call
  template <typename... Ts> auto _begin() {
    return getBegins(std::tuple<Ts...>{});
  }

  // Recursive case: handle the first type and recursively call for the
  // remaining types
  template <typename T, typename... Ts> auto getEnds(std::tuple<T, Ts...>) {
    if constexpr (sizeof...(Ts) > 0) {
      auto remaining = getEnds(std::tuple<Ts...>{});
      return std::tuple_cat(std::make_tuple(components_end<T>()), remaining);
    } else
      return std::make_tuple(components_end<T>());
  }

  // Wrapper function to initialize the recursive call
  template <typename... Ts> auto _end() { return getEnds(std::tuple<Ts...>{}); }

  /* template <typename T, typename... Ts> */
  /* auto unwrapIterators(std::tuple<T, Ts...> iterTuple) { */
  /*   const auto t = std::make_tuple(*std::get<T>(iterTuple)); */
  /*   if constexpr (sizeof...(Ts) > 0) { */
  /*     return std::tuple_cat(t, unwrapIterators<Ts...>(iterTuple)); */
  /*   } else { */
  /*     return t; */
  /*   } */
  /* } */

  template <typename... Ts, typename F> void forEach(F &&fn) {
    auto begin = _begin<Ts...>();
    auto end = _end<Ts...>();

    while (begin != end) {
      /* const auto all_some = std::apply( */
      /*     [](auto &&...args) { return std::all_of(((args.has_value()), ...));
       * }, */
      /*     begin); */

      /* if (all_some) { */
      std::apply(fn, begin);
      /* } */

      advance_all<sizeof...(Ts) - 1>(begin);
    }
  }

  // Recursive case: handle the first type and recursively call for the
  // remaining types
  template <unsigned int N, typename T, typename... Ts>
  void advance_all(std::tuple<T, Ts...> &iters) {
    std::get<N>(iters)++;

    if constexpr (N >= 1) {
      advance_all<N - 1, T, Ts...>(iters);
    }
  }
};

template <typename... Ts> struct Query {
  std::tuple<Ts...> beginnings, ends;

  auto begin() { return beginnings; }
  auto end() { return ends; }
};

} // namespace ecs

struct Foo {
  int x = 42;
};

struct Bar {
  std::string y = "bar";
};

template <typename T> using Iterator = typename std::vector<T>::iterator;

int main() {
  ecs::ECS ecs;

  const auto t1 = ecs.addEntity(Foo{}, Bar{});
  const auto t2 = ecs.addEntity(Foo{69}, Bar{"urmom"});

  ecs.forEach<Foo>([](auto f) {
    std::cout << f->x << " "
              << "" << '\n';

    f->x *= 2;
  });

  return 0;
}
