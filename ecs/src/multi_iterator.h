#pragma once

#include "archetype.h"
#include "defs.h"
#include <iterator>
#include <tuple>

namespace ecs {

// Given an archetype to iterate over (with component types {Ts...}),
// returns a tuple of iterators (act like pointers) to each entity in the archetype.
template <typename... Ts>
struct MultiIterator {
  using iterator_category = std::forward_iterator_tag;
  using element_type      = std::tuple<Iter<Ts>...>;
  using opt_value_type    = std::tuple<OptIter<Ts>...>;
  using pointer           = element_type *;
  using reference         = element_type &;
  using difference_type   = std::ptrdiff_t;

  MultiIterator(Archetype &a) : arch(a) { refHolder = arch.getComponentsAtOffset<Ts...>(0); }

  MultiIterator &operator++() {
    offset++;
    // https://stackoverflow.com/a/54053084
    // Advance all iterators in the tuple
    std::apply([](auto &&...iterTuple) { ((iterTuple++), ...); }, refHolder);
    return *this;
  }

  reference operator*() { return refHolder; }

  MultiIterator operator++(int) {
    auto temp = *this;
    ++(*this);
    return temp;
  }

  bool atEnd() const { return offset == arch.size() - 1; }

  friend bool operator==(const MultiIterator &a, const MultiIterator &b) {
    // Comparing the data of both archetypes is too expensive
    return (a.offset == b.offset) && (&a.arch == &b.arch);
  }

private:
  Archetype   &arch;
  u32          offset = 0;
  element_type refHolder;
};
} // namespace ecs
