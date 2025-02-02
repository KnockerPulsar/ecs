#pragma once

#include "archetype.h"
#include "chained_iterator.h"
#include "defs.h"
#include "multi_iterator.h"
#include "type_set.h"
#include <algorithm>
#include <utility>
#include <vector>

namespace ecs {

template <typename... Ts>
struct QueryView {
  QueryView(std::vector<MultiIterator<Ts...>> iters) : iters(iters) {}

  ChainedIterator<Ts...> begin() { return ChainedIterator(iters, 0); }
  ChainedIterator<Ts...> end() { return ChainedIterator(iters, iters.size()); }

private:
  std::vector<MultiIterator<Ts...>> iters;
};

class Archetypes {
public:
  // Check if any archetype _exactly_ matches the given typeset
  bool containsExact(const TypeSet &qts) const {
    return std::any_of(archetypes.cbegin(), archetypes.cend(), [&qts](const auto &pair) {
      const auto &ts = pair.first;
      return qts == ts;
    });
  }

  bool containsSubset(const TypeSet &qts) const {
    return std::any_of(archetypes.cbegin(), archetypes.cend(), [&qts](const auto &pair) {
      const auto &ts = pair.first;
      return qts.isSubsetOf(ts);
    });
  }

  void insert(const TypeSet &ts, Archetype arch) { archetypes.insert({ts, arch}); }

  Archetype &at(const TypeSet &ts) { return archetypes.at(ts); }

  void clear() { archetypes.clear(); }

  // Figure out what archetypes contain the given components as their subset
  // Then chain their component iterators to iterate over all of them
  //
  // For example if we have two archetypes {A, B, C} and {B, C, D}
  // and we get a query for {B, C}.
  template <typename... Ts>
  QueryView<Ts...> getQueryIter(Query<Ts...> q) {
    const auto qts   = TypeSet(q);
    auto       iters = std::vector<MultiIterator<Ts...>>();

    for (auto &[ts, arch] : archetypes) {
      if (qts.isSubsetOf(ts) && arch.size() != 0) {
        iters.push_back(MultiIterator<Ts...>(arch));
      }
    }

    return QueryView(iters);
  }

  auto begin() const { return archetypes.cbegin(); }

  auto end() const { return archetypes.cend(); }

  auto size() const { return archetypes.size(); }

  template <typename... Ts>
  void addEntity(Entity eid, Ts &&...comps) {
    const auto typeset = TypeSet({typeid(Ts)...});
    if (!containsExact(typeset)) {
      archetypes.emplace(typeset, Archetype::create<Ts...>());
    }
    auto& arch = archetypes.at(typeset);
    arch.addEntity(eid, std::forward<Ts>(comps)...);
    entityToArchetype.emplace(eid, arch);
  }

  void removeEntity(Entity eid) {
    if (!entityToArchetype.contains(eid)) {
      std::cerr << "Nonexistent entity\n";
      return;
    }

    auto& arch = entityToArchetype.at(eid);
    arch.removeEntity(eid);
    entityToArchetype.erase(eid);
  }

private:
  std::unordered_map<TypeSet, Archetype> archetypes;
  std::unordered_map<Entity, Archetype&> entityToArchetype;
};
} // namespace ecs
