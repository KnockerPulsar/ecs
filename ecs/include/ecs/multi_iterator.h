#pragma once

#include "component_container.h"
#include "defs.h"
#include "tuple_utils.h"

namespace ecs {

template <typename... Ts>
struct MultiIterator {

  struct iterator {
    using iterator_category = std::forward_iterator_tag;
    using element_type      = std::tuple<Iter<Ts>...>;
    using opt_value_type    = std::tuple<OptIter<Ts>...>;
    using pointer           = element_type *;
    using reference         = element_type &;
    using difference_type   = std::ptrdiff_t;

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
      offset++;

      while (offset < maxOffset) {
        auto compTuple = cc->getEntityComponents<Ts...>(offset);

        if (ecs::allSome(compTuple) && iterator::allSome(unwrapTuple(compTuple))) {
          break;
        }

        offset++;
      }

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

  private:
    uint                offset = 0, maxOffset = 0;
    ComponentContainer *cc = nullptr;
    element_type        refHolder;
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
    for (auto current = 0u; current != cc.numEntities; current++) {
      auto iters = cc.getEntityComponents<Ts...>(current);
      if (!allSome(iters)) {
        continue;
      }

      if (iterator::allSome(unwrapTuple(iters)))
        return current;
    }

    return 0;
  }

  // Need to find the last iterator to have all components so dereferencing works properly.
  // If none exist, we'll return what's equivalent to `begin()`
  static uint findValidEnd(ComponentContainer &cc) {
    // Should be called `firstAnyNoneAfterLastAllSome` but that's just too long
    auto current = static_cast<int>(cc.numEntities - 1);
    for (; current >= 0; current--) {
      auto iters = cc.getEntityComponents<Ts...>(current);

      // One of the Ts doesn't have a component vector (no entity has that component)
      if (!allSome(iters)) {
        continue;
      }

      // The current entity doesn't have all the requested components
      if (!iterator::allSome(unwrapTuple(iters))) {
        continue;
      }

      return current + 1;
    }

    return 1;
  }

  iterator _begin, _end;

  iterator begin() { return _begin; }
  iterator end() { return _end; }
};
} // namespace ecs
