#pragma once

#include "defs.h"

#include <any>
#include <cassert>
#include <cstddef>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ecs {
struct TypeErasedVector {
  std::any inner;

  struct {
    std::function<void(std::size_t)> removeElement;
  } operations;

  template <typename T>
  static auto create()
  {
    TypeErasedVector temp;
    temp.inner = std::make_any<std::vector<T>>();

    return temp;
  }

  template <typename T>
  std::vector<T>& asVectorOf() {
    return std::any_cast<std::vector<T> &>(inner);
  }

  template <typename T>
  void initOperations()
  {
    operations.removeElement = [this](std::size_t index){
      auto& vec = asVectorOf<T>();
      vec.erase(std::next(vec.begin(), index));
    };
  }

  template <typename T>
  std::size_t addElement(T&& element)
  {
      auto& vec = asVectorOf<T>();
      vec.emplace_back(element);

      return vec.size() - 1;
  }

  void removeElement(std::size_t index) { operations.removeElement(index); }
};

// TODO Instead of storing a "tuple" of component vectors, maybe store one
// vector of tuples?
//
// Would likely need re-architecting of the whole iteration code as well...
class Archetype {
public:
  template <typename... Ts>
  static Archetype create() {
    auto arch = Archetype();
    (arch.addVector<Ts>(), ...);

    return arch;
  }

  template <typename... Ts>
  void addEntity(Entity eid, Ts &&...comps) {
    (addComponent(std::forward<Ts>(comps)), ...);
    entityToIndex.emplace(eid, _size);
    _size++;
  }

  void removeEntity(Entity eid)
  {
    auto const index = entityToIndex.at(eid);
    for(auto& [_, compVec]: componentVectors)
      compVec.removeElement(index);
    entityToIndex.erase(eid);
  }

  // Given a type, get a reference to its component vector.
  template <typename T>
  std::vector<T>& getComponentVector() {
    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return componentVectors.at(typeid(T)).asVectorOf<T>();
  }

  template <typename T>
  void addComponent(T&& comp) {
    componentVectors.at(typeid(T)).addElement(std::forward<T>(comp));
  }

  template <typename T>
  Iter<T> getComponentIterAtOffset(u32 offset) {
    auto compVec = getComponentVector<T>();
    return compVec.begin() + offset;
  }

  template <typename... Ts>
  requires(sizeof...(Ts) > 0)
  std::tuple<Iter<Ts>...> getComponentsAtOffset(u32 offset) {
    return std::make_tuple(getComponentIterAtOffset<Ts>(offset)...);
  }

  std::size_t size() const { return _size; }

private:
  std::unordered_map<std::type_index, TypeErasedVector> componentVectors;
  std::unordered_map<Entity, std::size_t> entityToIndex;

  // I know that the component vectors each hold their own size, but it's
  // not possible to access their sizes without an any cast, which requires
  // knowing what type to cast to (at least the component type).
  std::size_t _size = 0;

  Archetype() = default;

  template <typename T>
  void addVector() {
    assert(!componentVectors.contains(typeid(T)));
    auto [vec, _] = componentVectors.emplace(typeid(T), TypeErasedVector::create<T>());

    // Have to init operations here since it captures a pointer to the
    // variable. If we do it in the static constructor, it captures a pointer
    // to the address on the stack, which is nuked when we return the type
    // erased vector
    vec->second.template initOperations<T>();
  }
};
} // namespace ecs
