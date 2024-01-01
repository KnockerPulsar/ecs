#pragma once

#include <optional>
#include <tuple>

namespace ecs {

// https://stackoverflow.com/a/29753388
template <int N, typename... Ts>
using Nth = typename std::tuple_element<N, std::tuple<Ts...>>::type;

// Needed to work around C++ template deduction shinanigans
// If it was a template function, the instantiation inside the else branch would be fn<Ts..., N+1>(...)
// Which is interpreted as fn<Ts={Ts..., N+1}, 0, sizeof...(Ts)> (N+1 is interpreted to be a part of the parameter
// pack) We can't move Ts to the end since NEnd depends on it being already defined. We can move it to the middle:
//  	template <unsigned int N = 0, typename ...Ts, unsigned int NEnd = sizeof...(Ts) - 1>
//  	foo(...) {...}
//
// Then it would be called as: foo<0, Ts...>(optTuple)
template <typename... Ts>
struct OptTupleUnwrapper {
  std::tuple<std::optional<Ts>...> &optTuple;

  // Helps deduce the template parameters
  OptTupleUnwrapper(std::tuple<std::optional<Ts>...> &ot) : optTuple(ot) {}

  // Converts from tuple<option<OptIter<T1>>, option<OptIter<T2>>, ...> to tuple<OptIter<T1>, OptIter<T2>, ...>
  template <unsigned int N = 0, unsigned int NEnd = sizeof...(Ts) - 1>
  auto apply() {
    using T = Nth<N, Ts...>;

    T          unwrapped(std::get<N>(optTuple).value());
    const auto t = std::make_tuple(unwrapped);

    if constexpr (N == NEnd) {
      return t;
    } else {
      return std::tuple_cat(t, apply<N + 1>());
    }
  }
};

// Convenience wrapper around OptTupleUnwrapper
template <typename... Ts>
std::tuple<Ts...> unwrapTuple(std::tuple<std::optional<Ts>...> tuple) {
  return OptTupleUnwrapper(tuple).apply();
}

// General case, given a tuple of optional stuff, check if all elements have a value
template <typename... Ts>
static bool allSome(std::tuple<std::optional<Ts>...> tuple) {
  return std::apply([](auto &&...args) { return (args.has_value() && ...); }, tuple);
}

} // namespace ecs
