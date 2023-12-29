#pragma once

#include "commands.h"
#include "component_container.h"
#include "defs.h"
#include "level.h"
#include "tuple_utils.h"

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

    uint                offset = 0, maxOffset = 0;
    ComponentContainer *cc = nullptr;
    value_type          refHolder;

    iterator() = default;
    iterator(uint offset, uint maxOffset, ComponentContainer *cc) : offset(offset), maxOffset(maxOffset), cc(cc) {}

    static bool allSome(const opt_value_type &iter) {
      return std::apply([](auto &&...args) { return (args->has_value() && ...); }, iter);
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
    void updateRefHolder(uint offset) {
      refHolder = unwrapIterators(unwrapTuple(cc->getEntityComponents<Ts...>(offset)));
    }

    iterator &operator++() {
      bool shouldContinue = true;
      do {
        offset++;
        auto compTuple = cc->getEntityComponents<Ts...>(offset);
        shouldContinue = !allSome(unwrapTuple(compTuple)) && offset != maxOffset;
      } while (shouldContinue);

      return *this;
    }

    reference operator*() {
      updateRefHolder(offset);
      return refHolder;
    }
    pointer operator->() {
      updateRefHolder(offset);
      return &refHolder;
    }

    iterator operator++(int) {
      auto temp = *this;
      (*this)++;
      return temp;
    }

    friend bool operator==(const iterator &a, const iterator &b) {
      return (a.offset == b.offset) && (a.maxOffset == b.maxOffset) && (a.cc == b.cc);
    }
  };

  MultiIterator(ComponentContainer &cc) {
    auto validBegin = findValidStart(cc);
    auto validEnd   = findValidEnd(cc);

    _begin = iterator(validBegin, validEnd, &cc);
    _end   = iterator(validEnd, validEnd, &cc);
  }

  // Need to find the first iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  static uint findValidStart(ComponentContainer &cc) {
    for (auto current = 0; current != cc.numEntities; current++) {
      if (iterator::allSome(unwrapTuple(cc.getEntityComponents<Ts...>(current))))
        return current;
    }

    return 0;
  }

  // Need to find the last iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  // TODO: could probably be faster to start at the end and return on the first `allSome`, needs `iterator::retreat_all`
  static uint findValidEnd(ComponentContainer &cc) {
    // Should be called `firstAnyNoneAfterLastAllSome` but that's just too long
    auto lastAllSome = 0;
    auto current     = 0;
    for (; current != cc.numEntities; current++) {
      if (iterator::allSome(unwrapTuple(cc.getEntityComponents<Ts...>(current))))
        lastAllSome = current;
    }

    return ++lastAllSome;
  }

  iterator _begin, _end;

  iterator begin() { return _begin; }
  iterator end() { return _end; }
};
} // namespace ecs
