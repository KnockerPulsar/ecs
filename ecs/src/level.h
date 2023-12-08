#pragma once

// clang-format off
#include "defs.h"
#include "resources.h"
#include "component_container.h"
#include "multi_iterator.h"
#include "commands.h"
// clang-format on

#include <algorithm>
#include <any>
#include <functional>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>

namespace ecs {

// Just so we have a new type
struct LevelResources : Resources {};

// Forward decl
struct GlobalResources;

// https://stackoverflow.com/a/29753388
template<int N, typename... Ts> 
using Nth = typename std::tuple_element<N, std::tuple<Ts...>>::type;

template<typename ...Ts>
concept NoResources = 
 !std::is_same_v<Nth<0, Ts...>, LevelResources>
&& !std::is_same_v<Nth<0, Ts...>, GlobalResources>
&& !std::is_same_v<Nth<1, Ts...>, LevelResources>
&& !std::is_same_v<Nth<1, Ts...>, GlobalResources>;

template <typename... Ts>
struct OptTupleUnwrapper;

struct Level {
  struct Transition {
    std::string           sourceLevel, destinationLevel;
    std::function<bool()> transitionCondition;
  };

  ComponentContainer                        components;
  std::vector<std::function<void()>>        systems;
  std::vector<std::function<void(Level &)>> setupSystems;
  LevelResources                            levelResources;
  GlobalResources&			    globalResources;	// Obtained from the ECS instance containing this level.
  bool 					    isSetup = false;

  Entity addEmptyEntity() {
    // Make space for the new entities
    for (auto &[typeId, any_vec] : components.component_vectors) {
      components.componentOperations[typeId].pushDummy();
    }

    return components.numEntities;
  }

  template <typename... Ts>
  Entity addEntity(Ts &&...comps) {
    components.addEntity(std::forward<Ts>(comps)...);

    return components.numEntities;
  }

  void removeEntity(Entity eid) {
    for (auto &[_, op] : components.componentOperations) {
      op.removeComponent(eid);
    }
  }

  template <typename T>
  std::optional<std::reference_wrapper<Vector<T>>> getComponentVector() {
    return components.getComponentVector<T>();
  }

  void copyComponents(Commands &cmd, Entity sourceId, Entity destId) {
    for (auto &[typeId, anyVec] : cmd.entitiesToAdd.component_vectors) {
      cmd.entitiesToAdd.componentOperations[typeId].moveComponent(components, sourceId, destId);
    }
  }

  void processCommands(Commands &cmd) {
    // Loops over all added entities, creates an empty one in the ECS storage,
    // then moves over all components from `entitiesToAdd` to the ECS storage.
    for (uint32_t sourceId = 0; sourceId < cmd.entitiesToAdd.numEntities; sourceId++) {
      auto destId = addEmptyEntity();
      copyComponents(cmd, sourceId, destId);
    }

    for (auto &&eid : cmd.entitiesToRemove) {
      removeEntity(eid);
    }
  }
  
  // Given a tuple of optional elements, returns true only if all elements have a value.
  template <typename ...Ts>
  static bool allComponentsExist(const  std::tuple<std::optional<OptIter<Ts>>...> &tuple) {
    return std::apply([](auto &&...args) { return (args.has_value() && ...); }, tuple);
  }
  
  // 3 possible inputs: Global resources, level resources, components
  //
  // Components
  // Level resources
  // Global resources
  //
  // Level resources + components
  // Global resources + components
  // Global resources + level resources
  // Global resourecs + level resources + components

  // Add system WITHOUT access to resources.
  template <typename... Ts, typename F>
  requires(NoResources<Ts...>) void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto begins = getBegins<Ts...>(*this);
      auto ends = getEnds<Ts...>(*this);

      if(allComponentsExist<Ts...>(begins)) {
	auto multiIter = MultiIterator<Ts...>{
	  OptTupleUnwrapper<Ts...>::apply(ends),
	  OptTupleUnwrapper<Ts...>::apply(begins)
	};
	std::invoke(fn, multiIter);
      } else {
	std::cerr << "Attempt to run a system with no available components (" << typeid(std::tuple<Ts...>).name() << ").\n";
      }
    });
  }

  // Add system with access to only to level resources.
  template <typename R, typename F>
  requires(std::is_same_v<LevelResources, R>) 
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      std::invoke(fn, std::ref(levelResources));
    });
  }
  
  // Add system with access to only to global resources.
  template <typename R, typename F>
  requires(std::is_same_v<GlobalResources, R>) void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      std::invoke(fn, std::ref(globalResources));
    });
  }

  // Add system with access to level resources + components.
  template <typename R, typename... Ts, typename F>
  requires(std::is_same_v<LevelResources, R> && sizeof...(Ts) > 0) 
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto begins = getBegins<Ts...>(*this);
      auto ends = getEnds<Ts...>(*this);

      if(allComponentsExist<Ts...>(begins)) {
	auto multiIter = MultiIterator<Ts...>{
	  OptTupleUnwrapper<Ts...>::apply(begins), 
	  OptTupleUnwrapper<Ts...>::apply(ends)
	};
	std::invoke(fn, std::ref(levelResources), multiIter);
      } else {
	std::cerr << "Attempt to run a system with no available components (" << typeid(std::tuple<Ts...>).name() << ").\n";
      }
    });
  }
  
  // Add system with access to global resources + components.
  template <typename R, typename... Ts, typename F>
  requires(std::is_same_v<GlobalResources, R> && sizeof...(Ts) > 0) 
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto begins = getBegins<Ts...>(*this);
      auto ends = getEnds<Ts...>(*this);

      if(allComponentsExist<Ts...>(begins)) {
	auto multiIter = MultiIterator<Ts...>{
	  OptTupleUnwrapper<Ts...>::apply(begins), 
	  OptTupleUnwrapper<Ts...>::apply(ends)
	};
	std::invoke(fn, std::ref(globalResources), multiIter);
      } else {
	std::cerr << "Attempt to run a system with no available components (" << typeid(std::tuple<Ts...>).name() << ").\n";
      }
    });
  }
  
  // Add system with access to global resources and level resources, but not components.
  template <typename R1, typename R2, typename F>
  requires(std::is_same_v<GlobalResources, R1> && std::is_same_v<LevelResources, R2>) 
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
	std::invoke(fn, std::ref(globalResources), std::ref(levelResources));
    });
  }
  
  // Add system with access to global resources, level resources, and components.
  template <typename R1, typename R2, typename... Ts, typename F>
  requires(std::is_same_v<GlobalResources, R1> && std::is_same_v<LevelResources, R2> && sizeof...(Ts) > 0) 
  void addSystem(F &&fn) {
    systems.push_back([this, fn]() {
      auto begins = getBegins<Ts...>(*this);
      auto ends = getEnds<Ts...>(*this);

      if(allComponentsExist<Ts...>(begins)) {
	auto multiIter = MultiIterator<Ts...>{
	  OptTupleUnwrapper<Ts...>::apply(begins), 
	  OptTupleUnwrapper<Ts...>::apply(ends)
	};
	std::invoke(fn, std::ref(globalResources), std::ref(levelResources), multiIter);
      } else {
	std::cerr << "Attempt to run a system with no available components (" << typeid(std::tuple<Ts...>).name() << ").\n";
      }
    });
  }

  template <typename F>
  void addSetupSystem(F &&fn) {
    setupSystems.push_back(fn);
  }

  void runSystems() {
    for (auto &sys : systems) { sys(); }

    if (auto cmd = levelResources.getResource<Commands>()) {
      processCommands(*cmd);
    }
  }

  void runSetupSystems() {
    for (auto &setupSys : setupSystems) {
      setupSys(*this);
    }
  }

  template <typename R>
  void addResource(R initialValue) {
    levelResources.addResource(initialValue);
  }
};

// Needed to work around C++ template deduction shinanigans
// If it was a template function, the instantiation inside the else branch would be fn<Ts..., N+1>(...)
// Which is interpreted as fn<Ts={Ts..., N+1}, 0, sizeof...(Ts)> (N+1 is interpreted to be a part of the parameter pack)
// We can't move Ts to the end since NEnd depends on it being already defined.
// We can move it to the middle:
//  	template <unsigned int N = 0, typename ...Ts, unsigned int NEnd = sizeof...(Ts) - 1>
//  	foo(...) {...}
//
// Then it would be called as: foo<0, Ts...>(optTuple)
template<typename ...Ts>
struct OptTupleUnwrapper {
  // Converts from tuple<option<OptIter<T1>>, option<OptIter<T2>>, ...> to tuple<OptIter<T1>, OptIter<T2>, ...>
  template <unsigned int N = 0, unsigned int NEnd = sizeof...(Ts) - 1>
  static auto apply(std::tuple<std::optional<OptIter<Ts>>...>& optTuple) {
    using T = typename std::tuple_element<N, std::tuple<Ts...>>::type;

    OptIter<T>    comp_it(std::get<N>(optTuple).value());
    const auto t = std::make_tuple(comp_it);

    if constexpr (N == NEnd) {
      return t;
    } else {
      return std::tuple_cat(t, apply<N + 1>(optTuple));
    }
  }
};

template <typename T2, typename... Ts2>
std::tuple<std::optional<OptIter<T2>>, std::optional<OptIter<Ts2>>...> 
getBegins(Level& lvl) {
  auto compVec = lvl.getComponentVector<T2>();

  std::tuple<std::optional<ecs::OptIter<T2>>> t_begin = 
    std::make_tuple(
	compVec.has_value()? 
	compVec->get().begin()
	: std::optional<ecs::OptIter<T2>>()
	);

  if constexpr (sizeof...(Ts2) > 0) {
    auto remaining = getBegins<Ts2...>(lvl);
    return std::tuple_cat(t_begin, remaining);
  } else {
    return t_begin;
  }
}

template <typename T2, typename... Ts2>
std::tuple<std::optional<OptIter<T2>>, std::optional<OptIter<Ts2>>...>
getEnds(Level& lvl) {
  auto compVec = lvl.getComponentVector<T2>();

  std::tuple<std::optional<ecs::OptIter<T2>>> t_end = 
	std::make_tuple(
	    compVec.has_value()? 
	      compVec->get().end()
	      : std::optional<ecs::OptIter<T2>>()
	 );
  if constexpr (sizeof...(Ts2) > 0) {
    auto remaining = getEnds<Ts2...>(lvl);
    return std::tuple_cat(t_end, remaining);
  } else {
    return t_end;
  }
}

} // namespace ecs
