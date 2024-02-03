#pragma once

#include "defs.h"

#include <cstddef>
#include <functional>
#include <set>
#include <typeindex>

namespace ecs {

class TypeSet {

public:
  explicit TypeSet(std::initializer_list<std::type_index> ts) : types(ts), hash(computeHash(ts)) {}

  template <typename... Ts>
  explicit TypeSet(Query<Ts...> /*DUMMY*/) : TypeSet({typeid(Ts)...}) {}

  auto cbegin() const { return types.cbegin(); }
  auto cend() const { return types.cend(); }

  bool operator==(const TypeSet &rhs) const { return hash == rhs.hash; }

  bool isSubsetOf(const TypeSet &ts) const { return std::includes(ts.cbegin(), ts.cend(), cbegin(), cend()); }

private:
  static std::size_t computeHash(std::initializer_list<std::type_index> ts) {
    std::size_t seed = 0;
    for (const auto &t : ts) {
      // Borrowed from boost
      seed ^= std::hash<std::type_index>{}(t) + std::size_t(0x9e3779b9) + (seed << 6) + (seed >> 2);
    }
    return seed;
  }

  const std::set<std::type_index> types;
  const size_t                    hash;
  friend std::hash<ecs::TypeSet>;
};

} // namespace ecs

namespace std {

template <>
struct hash<ecs::TypeSet> {
  size_t operator()(const ecs::TypeSet &x) const noexcept { return x.hash; }
};

} // namespace std
