#pragma once

#include "defs.h"

#include <any>
#include <functional>
#include <optional>
#include <typeindex>
#include <iostream>

namespace ecs {
struct ComponentContainer {
  struct ComponentOperation {
    std::function<void()>                                     pushDummy;
    std::function<void(ComponentContainer &, Entity, Entity)> moveComponent;
    std::function<void(Entity)>                               removeComponent;
  };

  friend class ECS;
  friend struct Commands;
  std::unordered_map<std::type_index, std::any> component_vectors;

  // Need something to map a type id to a std::vector<T>::push_back
  std::unordered_map<std::type_index, ComponentOperation> componentOperations;

  uint32_t numEntities = 0;

  template <typename T>
  std::optional<std::reference_wrapper<Vector<T>>> getComponentVector() {
    if(!component_vectors.contains(typeid(T))) {
      return {};
    }

    // NOTE: map::operator[] default constructs at whatever key you're looking
    // up if there's no value there. In this case, it constructs an empty
    // `std::any`
    return std::any_cast<Vector<T> &>(component_vectors[typeid(T)]);
  }

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    (lazilyInitComponentVector<Ts>(), ...);
    (lazilyRegisterComponentOperations<Ts>(), ...);

    for (auto &[type_id, any] : component_vectors) {
      componentOperations[type_id].pushDummy();
    }

    (addComponent(std::forward<Ts>(comps)), ...);

    numEntities += 1;
    return numEntities;
  }

private:
  template <typename T>
  void addComponent(T comp) {
    auto compVec = getComponentVector<T>();
    compVec.value().get()[numEntities] = comp; 
  }

  template <typename T>
  void lazilyInitComponentVector() {
    if(!component_vectors.contains(typeid(T))) {
      component_vectors.insert({typeid(T), std::make_any<Vector<T>>(numEntities)});
    } 
 }

  template <typename T>
  void lazilyRegisterComponentOperations() {
    if (auto f = componentOperations.find(typeid(T)); f == componentOperations.end()) {
      auto dummyPusher = [this]() {
	getComponentVector<T>().value().get().push_back(std::nullopt); 
      };

      auto moveComponent = [this](ComponentContainer &oth, Entity sourceId, Entity destId) {
        auto selfVec    = getComponentVector<T>();
        auto otherVec   = oth.getComponentVector<T>();

	// FIXME: Need to check if either vector doesn't exist
        otherVec.value().get()[destId] = std::move(selfVec.value().get()[sourceId]);
      };

      auto removeComponent = [this](Entity eid) { 
	auto compVec = getComponentVector<T>();
	if(compVec) {
	  compVec.value().get()[eid]= std::nullopt; 
	} else {
	  std::cerr << "Attempt to remove a component that doesn't exist.\n";
	}
      };

      componentOperations.insert({typeid(T), ComponentOperation{dummyPusher, moveComponent, removeComponent}});
    }
  }
};
} // namespace ecs
