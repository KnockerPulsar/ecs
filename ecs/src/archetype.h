#pragma once

#include "defs.h"

#include <any>
#include <cassert>
#include <cstddef>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace ecs {
class Archetype {
public:
  template <typename... Ts>
  static Archetype create() {
    auto arch = Archetype();
    (arch.addVector<Ts>(), ...);

    return arch;
  }

  template <typename... Ts>
  void addEntity(Ts &&...comps) {
    (addComponent(comps), ...);
    _size++;
  }

  // Given a type, get a reference to its component vector.
  template <typename T>
  std::reference_wrapper<std::vector<T>> getComponentVector() {
    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<std::vector<T> &>(componentVectors.at(typeid(T)));
  }

  template <typename T>
  void addComponent(T comp) {
    getComponentVector<T>().get().push_back(comp);
  }

  template <typename T>
  Iter<T> getComponentIterAtOffset(u32 offset) {
    auto compVec = getComponentVector<T>();
    return compVec.get().begin() + offset;
  }

  template <typename... Ts>
  requires(sizeof...(Ts) > 0)
  std::tuple<Iter<Ts>...> getComponentsAtOffset(u32 offset) {
    return std::make_tuple(getComponentIterAtOffset<Ts>(offset)...);
  }

  std::size_t size() const { return _size; }

private:
  std::unordered_map<std::type_index, std::any> componentVectors;

  // I know that the component vectors each hold their own size, but it's
  // not possible to access their sizes without an any cast, which requires
  // knowing what type to cast to (at least the component type).
  std::size_t _size = 0;

  Archetype() = default;

  template <typename T>
  void addVector() {
    assert(!componentVectors.contains(typeid(T)));
    componentVectors.insert({typeid(T), std::make_any<std::vector<T>>()});
  }
};
} // namespace ecs
