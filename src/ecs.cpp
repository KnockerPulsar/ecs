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

struct Foo {
  int x = 42;
};

struct Bar {
  std::string y = "bar";
};

namespace ecs {
using Entity = std::uint32_t;

template <typename T> using Iterator = typename std::vector<T>::iterator;

class ECS {
  std::unordered_map<std::type_index, std::any> component_vectors;

  template <typename... Ts> struct MultiIterator {
    ECS &owner;
    std::tuple<Iterator<Ts>...> begins, ends;

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

    template <typename F> void forEach(F &&fn) {
      auto current = begins;

      while (current != ends) {
        /* const auto all_some = std::apply( */
        /*     [](auto &&...args) { return std::all_of(((args.has_value()),
         * ...));
         * }, */
        /*     begin); */

        /* if (all_some) { */
        std::apply(fn, current);
        /* } */

        advance_all<sizeof...(Ts) - 1>(current);
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

  template <typename... Ts, typename F> void forEach(F &&fn) {
    MultiIterator<Ts...>(*this).forEach(fn);
  }
};

} // namespace ecs

int main() {
  ecs::ECS ecs;

  const auto t1 = ecs.addEntity(Foo{}, Bar{});
  const auto t2 = ecs.addEntity(Foo{69}, Bar{"urmom"});

  ecs.forEach<Foo, Bar>([](ecs::Iterator<Foo> f, ecs::Iterator<Bar> b) {
    std::cout << f->x << " " << b->y << '\n';
  });

  return 0;
}
