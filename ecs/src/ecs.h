#include "defs.h"
#include "level.h"
#include <unordered_map>

namespace ecs {
using LevelMap = std::unordered_map<std::string, Level>;
class ECS {
  LevelMap                       levels;
  std::vector<Level::Transition> levelTransitions;
  std::string                    startLevel, _currentLevel;

public:
  void addEmptyLevel(const std::string &levelName) { levels.insert({levelName, Level{}}); }

  template <typename F>
  void addSetupSystem(const std::string &levelName, F &&fn) {
    levels.find(levelName)->second.addSetupSystem(fn);
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
        std::cout << "transition\n";
        _currentLevel = transition.destinationLevel;
      }
    }
  }

  inline Level &currentLevel() { return levels.find(_currentLevel)->second; }

  void runSystems() { return currentLevel().runSystems(); }
  void runSetupSystems() { return currentLevel().runSetupSystems(); }
};
} // namespace ecs
