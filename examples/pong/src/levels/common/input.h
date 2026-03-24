#pragma once

#include "common/res.h"
#include "defs.h"
#include "resources.h"

#include <raylib.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <queue>
#include <ranges>
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
      using namespace std::ranges;

      auto const keyDown = [](auto const index_key) {
        auto [_, key] = index_key;
        return key == true;
      };

      auto const indexToString = [](auto const index_key) {
        return std::to_string(std::get<0>(index_key));
      };

      return state.frameKeysDown               //
             | views::enumerate                //
             | views::filter(keyDown)          //
             | views::transform(indexToString) //
             | views::join_with(' ')           //
             | to<std::string>();
    }

    friend std::ostream &operator<<(std::ostream &ostream, State const &state) {
      ostream << to_string(state) << '\n';
      return ostream;
    }

    friend std::istream &operator>>(std::istream &istream, State &state) {
      size_t index;
      while (istream >> index) {
        state.frameKeysDown.at(index) = true;
      }

      return istream;
    }

    friend bool operator<=>(State const &lhs, State const &rhs) = default;
  };

  using FrameInputState = std::tuple<DeltaTime, Input::State>;

  Input(std::optional<InputType> type, std::filesystem::path path) : inputType(type), filepath(path) {
    if (type == InputType::playback) {
      readFromFile(path);
    }
  }

  ~Input() {
    if (inputType && *inputType == InputType::record) {
      writeToFile(filepath);
    }
  }

  static void pollNewInputs(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();
    auto &dt    = global.getResource<DeltaTime>()->get();

    State const prevState = input.state;
    State newState;
    for (u32 i = 0; i < newState.frameKeysDown.size(); i++) {
      newState.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
    }

    if (auto const it = input.inputType) {
      if (it == InputType::playback) {
        if (!input.inputStates.empty()) {
          auto const [_, frameState] = input.inputStates.front();
          input.inputStates.pop();

          newState = frameState;
        }
      } else {
        input.inputStates.push({dt, newState});
      }
    }

    input.prevState = prevState;
    input.state = newState;
  }

  static void onFrameEnd(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();

    input.prevState = input.state;
  }

  bool isKeyDown(KeyboardKey k) const { return state.frameKeysDown[static_cast<u32>(k)]; }

  bool wasKeyPressed(KeyboardKey k) const {
    return prevState.frameKeysDown[static_cast<u32>(k)] && !state.frameKeysDown[static_cast<u32>(k)];
  }

  std::optional<FrameInputState> getCurrentInputState() {
    if (
      auto const it = inputType;
      it && it == InputType::playback && !inputStates.empty()
    ) {
      auto const ret = inputStates.front();
      return ret;
    }

    return std::nullopt;
  }

  // TODO should be private
  void writeToFile(std::filesystem::path filePath) {
    std::ofstream outputFile{filePath};

    while(!inputStates.empty())
    {
      auto const [dt, state] = inputStates.front();
      inputStates.pop();

      outputFile << std::format("{} {}\n", dt.value, to_string(state));
    }
  }

  // TODO should be private
  void readFromFile(std::filesystem::path filePath) {
    std::ifstream inputFile{filePath};

    std::string line;
    while (std::getline(inputFile, line)) {
      std::stringstream ss{line};

      DeltaTime    dt;
      Input::State state;

      ss >> dt >>  state;

      inputStates.push({dt, state});
    }
  }

private:
  std::queue<FrameInputState> inputStates;
  State                                     state, prevState;
  std::optional<InputType> const            inputType;
  std::filesystem::path const               filepath;
};
} // namespace pong
