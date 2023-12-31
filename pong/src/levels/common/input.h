#pragma once

#include "common/res.h"
#include "defs.h"
#include "resources.h"

#include <raylib.h>

#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

namespace pong {
struct Input {
  struct State {
    std::array<bool, 348> frameKeysDown, prevFrameKeysDown;

    State() {
      for (auto &k : frameKeysDown)
        k = false;
      for (auto &k : prevFrameKeysDown)
        k = false;
    }
  };

  static void pollNewInputs(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();

    for (u32 i = 0; i < input.state.frameKeysDown.size(); i++) {
      input.state.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
    }
  }

  static void onFrameEnd(ecs::Resources &global) {
    auto &input = global.getResource<Input>()->get();

    input.state.prevFrameKeysDown = input.state.frameKeysDown;
    input.state.frameKeysDown     = {};
  }

  bool isKeyDown(KeyboardKey k) const { return state.frameKeysDown[static_cast<u32>(k)]; }

  bool wasKeyPressed(KeyboardKey k) const {
    return state.frameKeysDown[static_cast<u32>(k)] && !state.prevFrameKeysDown[static_cast<u32>(k)];
  }

private:
  // Raylib's `KeyboardKey` enum has a maximum value of 348.
  State state;
};
} // namespace pong
