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

    iterator() = default;
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

    // Converts from tuple<OptIter<T1>, OptIter<T2>, ...> to tuple<Iter<T1>, Iter<T2>, ...>
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
      if(current != end) {
	currentForInc = unwrapIterators(current);
      }
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

  MultiIterator(iterator::opt_value_type begin, iterator::opt_value_type end) {
    auto validBegin = findValidStart(begin, end);
    auto validEnd = findValidEnd(begin, end);

    _begin = iterator(validBegin, validEnd);
    _end = iterator(validEnd, validEnd);
  }

  // Need to find the first iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  static std::tuple<OptIter<Ts>...> findValidStart(iterator::opt_value_type begin, iterator::opt_value_type end) {
    for (auto current = begin; current != end; iterator::advance_all(current)) {
      if (iterator::allSome(current))
        return current;
    }

    return begin;
  }

  // Need to find the last iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  // TODO: could probably be faster to start at the end and return on the first `allSome`, needs `iterator::retreat_all`
  static std::tuple<OptIter<Ts>...> findValidEnd(iterator::opt_value_type begin, iterator::opt_value_type end) {
    auto lastAllSome = begin;   // Should be called `firstAnyNoneAfterLastAllSome` but that's just too long
    auto current     = begin;
    for (; current != end; iterator::advance_all(current)) {
      if (iterator::allSome(current))
        lastAllSome = current;
    }

    iterator::advance_all(lastAllSome);

    return lastAllSome;
  }

  iterator _begin, _end;

  iterator begin() { return _begin; }
  iterator end() { return _end; }
};
} // namespace ecs
