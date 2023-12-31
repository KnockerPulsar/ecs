#pragma once

#include "level.h"
#include "resources.h"

#include <cassert>
#include <functional>
#include <unordered_map>

namespace ecs {
using LevelMap = std::unordered_map<std::string, Level>;

// Should be used to indicate that the main loop should exit
struct Quit {};

struct TransitionToScene {
  std::string levelName;
};

class ECS {
  LevelMap    levels;
  std::string startLevel, _currentLevel;

  std::vector<std::function<void()>> globalResourceSystemsPre;  // Pre-frame systems
  std::vector<std::function<void()>> globalResourceSystemsPost; // Post-frame systems

public:
  Resources                          globalResources;           // Resources shared between all levels.
                                                                
  template <typename... F>
  void addStartupLevel(const std::string &levelName, F &&...setupFunctions) {
    startLevel = _currentLevel = levelName;
    addLevel(levelName, setupFunctions...);
  }

  template <typename... F>
  void addLevel(const std::string &levelName, F &&...setupFunctions) {
    auto [iter, inserted] = levels.emplace(levelName, Level{.globalResources = globalResources});
    assert(inserted);

    (iter->second.addSetupSystem(setupFunctions), ...);
  }

  // Just some code to run when setting up a level
  template <typename F>
  void addSetupSystem(const std::string &levelName, F &&fn) {
    levels.find(levelName)->second.addSetupSystem(fn);
  }

  void transitionToLevel(std::string levelName) {
    std::cout << "Transitioning to level " << levelName << " from level " << _currentLevel << '\n';
    runResetSystems();
    _currentLevel = levelName;
    runSetupSystems();
  }

  void checkTransitions() {
    if (auto transition = globalResources.consumeResource<TransitionToScene>()) {
      if (!levels.contains(transition->levelName)) {
        std::cerr << "Attempting to transition to non-existent scene: " << transition->levelName << '\n';
        std::cerr << "Make sure the name is correct and that it has been added to the ECS instance\n";
        std::terminate();
      }

      transitionToLevel(transition->levelName);
      return;
    }
  }

  inline Level &currentLevel() { return levels.find(_currentLevel)->second; }

  void runResetSystems() { currentLevel().runResetSystems(); }
  void runPerFrameSystems() { currentLevel().runSystems(); }

  void runSetupSystems() {
    if (_currentLevel.empty()) {
      std::cerr << "No startup level was given\n";
      std::terminate();
    }

    currentLevel().runSetupSystems();
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
    globalResources.addResource(std::forward<R>(resource));
  }

  template <typename F>
  requires(MatchSignature<F, void, Resources &>)
  void addGlobalResourceSystemPre(F &&sys) {
    globalResourceSystemsPre.push_back([this, sys]() { std::invoke(sys, std::ref(globalResources)); });
  }

  template <typename F>
  requires(MatchSignature<F, void, Resources &>)
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

  bool shouldQuit() { return globalResources.getResource<Quit>().has_value(); }
};
} // namespace ecs
