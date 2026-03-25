#pragma once

#include "multi_iterator.h"
namespace ecs {

// Chains the iteration of multiple MultiIterators together
template <typename... Ts>
class ChainedIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using element_type      = std::tuple<Iter<Ts>...>;
  using opt_value_type    = std::tuple<OptIter<Ts>...>;
  using pointer           = element_type *;
  using reference         = element_type &;
  using difference_type   = std::ptrdiff_t;

  ChainedIterator(std::vector<MultiIterator<Ts...>> &_iters, u32 currentIter)
      : iters(_iters), currentIter(currentIter) {}

  ChainedIterator &operator++() {
    if (iters[currentIter].atEnd()) {
      currentIter++;
    } else {
      iters[currentIter]++;
    }

    return *this;
  }

  reference operator*() { return *iters[currentIter]; }

  ChainedIterator operator++(int) {
    auto temp = *this;
    (*this)++;
    return temp;
  }

  friend bool operator==(const ChainedIterator &a, const ChainedIterator &b) {
    // Comparing the data of both archetypes is too expensive
    return (a.currentIter == b.currentIter) && (a.iters == b.iters);
  }

  auto begin() { return ChainedIterator(iters, 0); }

  auto end() { return ChainedIterator(iters, iters.size()); }

private:
  std::vector<MultiIterator<Ts...>> &iters;
  u32                                currentIter = 0;
};

} // namespace ecs
