#pragma once

#include "defs.h"
#include "ecs.h"
#include "level.h"
#include "raylib.h"

#include <cmath>

namespace pong {
using Time      = f32;
using DeltaTime = f32;

struct Renderer {
  struct RenderTextCommand {
    u32              x, y;
    std::string_view text;
    Color            color;
    u32              fontSize;
  };
  struct RenderRectCommand {
    u32   x, y, width, height;
    Color color;
  };

  std::vector<RenderTextCommand> textCommands;
  std::vector<RenderRectCommand> rectCommands;

  static void system(ecs::GlobalResources &r) {
    auto &self = r.getResource<Renderer>()->get();

    BeginDrawing();
    ClearBackground(BLACK);

    for (const auto &tc : self.textCommands) {
      DrawText(tc.text.data(), tc.x, tc.y, tc.fontSize, tc.color);
    }
    self.textCommands.clear();

    for (const auto &rc : self.rectCommands) {
      DrawRectangle(rc.x, rc.y, rc.width, rc.height, rc.color);
    }
    self.rectCommands.clear();

    EndDrawing();
  }
};

struct Text {
  std::string text;
  Color       color;

  float fontScale = 1.0;
  u32   x         = 0;
  u32   y         = 0;
  u32   baseSize  = 80;

  static u32 alignTextToCenter(const Text &tt) {
    const auto offset = MeasureText(tt.text.c_str(), tt.fontScale * tt.baseSize) / 2;
    return tt.x - offset;
  }

  [[nodiscard]] Renderer::RenderTextCommand drawCenterAligned() const {
    return Renderer::RenderTextCommand{
        .x        = alignTextToCenter(*this),
        .y        = y,
        .text     = text,
        .color    = color,
        .fontSize = static_cast<u32>(baseSize * fontScale),
    };
  }
};

struct TextAnimation {
  std::function<void(ecs::GlobalResources &, ecs::Iter<Text>, float)> animate;
  float                                                               animationSpeed;
};

void sinAnimation(ecs::GlobalResources &r, ecs::Iter<Text> t, float animationSpeed) {
  auto &time   = r.getResource<Time>()->get();
  t->fontScale = (std::sin(time * animationSpeed)) / 4 + 0.75;
}

struct CenterTextAnchor {};

struct Input {
  Input() {
    for (auto &f : frameKeysDown) {
      f = false;
    }
    for (auto &f : prevFrameKeysDown) {
      f = false;
    }
  }

  static void pollNewInputs(ecs::GlobalResources &r) {
    auto &input = r.getResource<Input>().value().get();

    for (u32 i = 0; i < input.frameKeysDown.size(); i++) {
      input.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
    }
  }

  static void onFrameEnd(ecs::GlobalResources &r) {
    auto &input             = r.getResource<Input>().value().get();
    input.prevFrameKeysDown = input.frameKeysDown;

    for (auto &f : input.frameKeysDown) {
      f = false;
    }
  }

  bool isKeyDown(KeyboardKey k) const { return frameKeysDown[static_cast<u32>(k)]; }

  bool wasKeyPressed(KeyboardKey k) const {
    return frameKeysDown[static_cast<u32>(k)] && !prevFrameKeysDown[static_cast<u32>(k)];
  }

private:
  // Raylib's `KeyboardKey` enum has a maximum value of 348.
  std::array<bool, 348> frameKeysDown, prevFrameKeysDown;
};

} // namespace pong
