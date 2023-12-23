#pragma once

#include "defs.h"
#include "level.h"
#include "resources.h"
#include <functional>
#include <type_traits>
#include <unordered_map>

namespace ecs {
using LevelMap = std::unordered_map<std::string, Level>;

struct GlobalResources : Resources {};

template <typename F, typename R, typename... Args>
concept MatchSignature = std::is_invocable_v<F, Args...> && std::is_invocable_r_v<R, F, Args...>;

class ECS {
  LevelMap                       levels;
  std::vector<Level::Transition> levelTransitions;
  std::string                    startLevel, _currentLevel;

  GlobalResources                    globalResources;           // Resources shared between all levels.
  std::vector<std::function<void()>> globalResourceSystemsPre;  // Pre-frame systems
  std::vector<std::function<void()>> globalResourceSystemsPost; // Post-frame systems

public:
  void addEmptyLevel(const std::string &levelName) {
    levels.insert({levelName, Level{.globalResources = globalResources}});
  }

  // Just some code to run when setting up a level
  template <typename F>
  void addSetupSystem(const std::string &levelName, F &&fn) {
    levels.find(levelName)->second.addSetupSystem(fn);
  }

  // A setup system that uses global resources
  // For example, that reads screen width and height.
  template <typename F>
  void addSetupSystemRes(const std::string &levelName, F &&fn) {
    levels.find(levelName)->second.addSetupSystem([this, &fn](ecs::Level &l) { fn(this->globalResources, l); });
  }

  void addTransition(Level::Transition &&lt) { levelTransitions.push_back(std::move(lt)); }

  bool validateLevelTransitions() const {
    if (startLevel.empty()) {
      return false;
    }

    return std::all_of(levelTransitions.begin(), levelTransitions.end(), [this](auto &transition) {
      const bool sourceValid      = levels.contains(transition.sourceLevel);
      const bool destinationValid = levels.contains(transition.destinationLevel);
      return sourceValid && destinationValid;
    });
  }

  bool setStartLevel(const std::string &startingLevelName) {
    if (!levels.contains(startingLevelName)) {
      return false;
    }

    startLevel = _currentLevel = startingLevelName;
    return true;
  }

  void checkTransitions() {
    for (const auto &transition : levelTransitions) {
      if (transition.sourceLevel != _currentLevel)
        continue;

      const bool shouldTransition = transition.transitionCondition();
      if (shouldTransition) {
        // TODO: cleanup source and init destination?
        _currentLevel = transition.destinationLevel;
        runSetupSystems();
      }
    }
  }

  inline Level &currentLevel() { return levels.find(_currentLevel)->second; }

  void runSystems() { return currentLevel().runSystems(); }
  void runSetupSystems() {
    return currentLevel().runSetupSystems();
    currentLevel().isSetup = true;
  }

  std::optional<std::reference_wrapper<Level>> getLevel(const std::string &levelName) {
    if (!levels.contains(levelName)) {
      return {};
    }

    return levels.find(levelName)->second;
  }

  // To add global resources while setting up scenes.
  template <typename R>
  void addGlobalResource(R &&resource) {
    globalResources.addResource(resource);
  }

  template <typename F>
    requires(MatchSignature<F, void, GlobalResources &>)
  void addGlobalResourceSystemPre(F &&sys) {
    globalResourceSystemsPre.push_back([this, sys]() { std::invoke(sys, std::ref(globalResources)); });
  }

  template <typename F>
    requires(MatchSignature<F, void, GlobalResources &>)
  void addGlobalResourceSystemPost(F &&sys) {
    globalResourceSystemsPost.push_back([this, sys]() { std::invoke(sys, std::ref(globalResources)); });
  }

  void runPreSystems() {
    for (auto &sys : globalResourceSystemsPre)
      sys();
  }
  void runPostSystems() {
    for (auto &sys : globalResourceSystemsPost)
      sys();
  }
};
} // namespace ecs
