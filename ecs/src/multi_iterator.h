#pragma once

#include "./commands.h"
#include "./defs.h"

#include <algorithm>
#include <cassert>
#include <functional>

namespace ecs {
template <typename... Ts>
struct MultiIterator {

  struct iterator {
    using opt_value_type = std::tuple<OptIter<Ts>...>;
    using value_type     = std::tuple<Iter<Ts>...>;
    using pointer        = value_type *;
    using reference      = value_type &;

    opt_value_type current, end;
    value_type     currentForInc; // Needed since we return a reference

    iterator(opt_value_type begin, opt_value_type end) : current(begin), end(end) {}

    static bool allSome(const opt_value_type &iter) {
      return std::apply([](auto &&...args) { return (args->has_value() && ...); }, iter);
    }

    template <unsigned int N = sizeof...(Ts) - 1>
    static void advance_all(opt_value_type &iters) {
      std::get<N>(iters)++;

      if constexpr (N >= 1) {
        advance_all<N - 1>(iters);
      }
    }

    template <unsigned int N = 0, unsigned int NEnd = sizeof...(Ts) - 1>
    static auto unwrapIterators(opt_value_type iterTuple) {
      using T = typename std::tuple_element<N, std::tuple<Ts...>>::type;
      Iter<T>    comp_it(&(std::get<N>(iterTuple)->value()));
      const auto t = std::make_tuple(comp_it);

      if constexpr (N == NEnd) {
        return t;
      } else {
        return std::tuple_cat(t, unwrapIterators<N + 1>(iterTuple));
      }
    }

    iterator &operator++() {
      do {
        advance_all(current);
      } while (!allSome(current) && current != end);
      return *this;
    }

    reference operator*() {
      currentForInc = unwrapIterators(current);
      return currentForInc;
    }

    pointer operator->() { return &unwrapIterators(current); }

    iterator operator++(int) {
      auto temp = *this;
      (*this)++;
      return temp;
    }

    friend bool operator==(const iterator &a, const iterator &b) { return a.current == b.current && a.end == b.end; }
  };

  MultiIterator(iterator::opt_value_type begin, iterator::opt_value_type end)
      : _begin(prepareBegin(begin, end)), _end(prepareEnd(begin, end)) {}

  // Need to find the first iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  static iterator prepareBegin(iterator::opt_value_type begin, iterator::opt_value_type end) {
    for (auto current = begin; current != end; iterator::advance_all(current)) {
      if (iterator::allSome(current))
        return iterator(current, end);
    }

    return iterator(begin, end);
  }

  // Need to find the last iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  // TODO: could probably be faster to start at the end and return on the first `allSome`, needs `iterator::retreat_all`
  static iterator prepareEnd(iterator::opt_value_type begin, iterator::opt_value_type end) {
    auto lastAnyNone = end;   // Should be called `firstAnyNoneAfterLastAllSome` but that's just too long
    auto lagger      = begin; // One step behind
    auto current     = begin;
    iterator::advance_all(current);
    for (; current != end; iterator::advance_all(current), iterator::advance_all(lagger)) {
      if (iterator::allSome(lagger) && !iterator::allSome(current))
        lastAnyNone = current;
    }

    return iterator(lastAnyNone, end);
  }

  iterator _begin, _end;

  iterator begin() { return _begin; }
  iterator end() { return _end; }
};
} // namespace ecs
