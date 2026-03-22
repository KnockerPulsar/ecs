#pragma once

#include "common/res.h"
#include "defs.h"
#include "resources.h"

#include <chrono>
#include <filesystem>
#include <ratio>
#include <raylib.h>

#include <array>
#include <queue>
#include <fstream>
#include <format>
#include <sstream>
#include <string>

namespace pong {
struct Input {
  enum class InputType : std::uint8_t { playback, record };
  using TimeStamp = std::chrono::nanoseconds;

  struct State {
    // Raylib's `KeyboardKey` enum has a maximum value of 348.
    std::array<bool, 348> frameKeysDown;

    State() {
      for (auto &k : frameKeysDown)
        k = false;
    }

    friend std::string to_string(State const &state) {
      std::stringstream ss;

      for (const auto *iter = state.frameKeysDown.begin(); iter != state.frameKeysDown.end(); iter++) {
        ss << *iter;

        if (std::next(iter) != state.frameKeysDown.end())
          ss << ' ';
      }

      return ss.str();
    }

    friend std::ostream &operator<<(std::ostream &ostream, State const &state) {
      ostream << to_string(state) << '\n';
      return ostream;
    }

    friend std::istream &operator>>(std::istream &istream, State &state) {
      for (auto& k: state.frameKeysDown) {
        istream >> k;
      }

      return istream;
    }

    friend bool operator<=>(State const &lhs, State const &rhs) = default;
  };

  Input(InputType type, std::filesystem::path path) : inputType(type), filepath(path) {
    if (type == InputType::playback) {
      readFromFile(path);
    }
  }

  ~Input() {
    if (inputType == InputType::record) {
      writeToFile(filepath);
    }
  }

  static void pollNewInputs(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();
    auto &frame  = global.getResource<Frame>()->get();
    if (input.inputType == InputType::record) {
      State newState;
      for (u32 i = 0; i < newState.frameKeysDown.size(); i++) {
        newState.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
      }

      if(newState != input.state) {
        input.inputStates.push({frame, newState});
        /* input.prevState = input.state; */
        input.state     = newState;
      }
    } else {
      if (auto state = input.getState(frame)) {
        input.prevState = input.state;
        input.state     = *state;
      }
    }
  }

  static void onFrameEnd(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();

    input.prevState = input.state;
  }

  bool isKeyDown(KeyboardKey k) const { return state.frameKeysDown[static_cast<u32>(k)]; }

  bool wasKeyPressed(KeyboardKey k) const {
    return prevState.frameKeysDown[static_cast<u32>(k)] && !state.frameKeysDown[static_cast<u32>(k)];
  }

  State getState() const { return state; }

  void recordInputs(Frame frame, Input::State const &state) { inputStates.push({frame, state}); }

  // Can we return multiple states in case the a frame takes too long and there
  // are multiple states with time < current time?
  std::optional<Input::State> getState(Frame frame) {
    if (inputStates.empty() || frame < inputStates.front().first)
      return {};

    auto const [_, retState] = inputStates.front();
    inputStates.pop();

    return retState;
  }

  void writeToFile(std::filesystem::path filePath) {
    std::ofstream outputFile{filePath};

    while(!inputStates.empty())
    {
      auto const [frame, state] = inputStates.front();
      inputStates.pop();

      outputFile << std::format("{:<16} {}\n", frame.value, to_string(state));
    }
  }

  void readFromFile(std::filesystem::path filePath) {
    std::ifstream inputFile{filePath};

    while (!inputFile.eof()) {
      Frame        frame;
      Input::State state;
      inputFile >> frame >> state;

      inputStates.push({frame, state});
    }
  }

private:
  std::queue<std::pair<Frame, Input::State>> inputStates;
  State                                     state, prevState;
  InputType const                           inputType;
  std::filesystem::path const               filepath;
};
} // namespace pong
