#pragma once

#include "defs.h"
#include "ecs.h"
#include "level.h"
#include "raylib.h"
#include "resources.h"

#include <cmath>

namespace pong {

const auto selectedOptionColor = MAGENTA;
const auto defaultTextColor    = DARKGRAY;

namespace sceneNames {
  const std::string mainMenu = "main-menu";
  const std::string mainGame = "main-game";
  const std::string gameOver = "game-over";
} // namespace sceneNames

template <typename T>
struct Wrapper {
  T value;

  Wrapper() = default;
  Wrapper(T &&v) : value(std::move(v)) {}
  Wrapper(const T &v) : value(v) {}

  // Allow automatic conversion to wrapped value
  operator const T &() const { return value; }
  operator T &() { return value; }
};

struct Time : public Wrapper<f32> {};
struct DeltaTime : public Wrapper<f32> {};
struct ScreenWidth : public Wrapper<u32> {};
struct ScreenHeight : public Wrapper<u32> {};
enum class GameResult : u8 { Won, Lost };

struct Renderer {
  struct TextCommand {
    u32              x, y;
    std::string_view text;
    Color            color;
    u32              fontSize;
  };

  static void system(ecs::Resources &global) {
    auto &self = global.getResource<Renderer>()->get();

    BeginDrawing();
    BeginBlendMode(BLEND_ALPHA);
    ClearBackground(BLACK);

    for (const auto &rc : self.rectCommands) {
      DrawRectangle(rc.x, rc.y, rc.width, rc.height, rc.color);
    }
    self.rectCommands.clear();

    for (const auto &rs : self.circleCommands) {
      DrawCircle(rs.x, rs.y, rs.radius, rs.color);
    }
    self.circleCommands.clear();

    for (const auto &tc : self.textCommands) {
      DrawText(tc.text.data(), tc.x, tc.y, tc.fontSize, tc.color);
    }
    self.textCommands.clear();

    EndBlendMode();
    EndDrawing();
  }

  void drawText(const TextCommand &tc) { textCommands.push_back(tc); }

  void drawText(u32 x, u32 y, std::string_view text, Color color, u32 fontSize) {
    textCommands.push_back(TextCommand{x, y, text, color, fontSize});
  }

  void drawRect(u32 x, u32 y, u32 width, u32 height, Color color) {
    rectCommands.push_back(RectCommand{x, y, width, height, color});
  }

  void drawCircle(u32 x, u32 y, u32 radius, Color color) {
    circleCommands.push_back(CircleCommand{x, y, radius, color});
  }

private:
  struct RectCommand {
    u32   x, y, width, height;
    Color color;
  };

  struct CircleCommand {
    u32   x, y, radius;
    Color color;
  };

  std::vector<TextCommand>   textCommands;
  std::vector<RectCommand>   rectCommands;
  std::vector<CircleCommand> circleCommands;
};

struct Text {
  std::string text;

  Color color     = defaultTextColor;
  float fontScale = 1.0;
  u32   x         = 0;
  u32   y         = 0;
  u32   baseSize  = 40;

  static u32 alignTextToCenter(const Text &tt) {
    const auto offset = MeasureText(tt.text.c_str(), tt.fontScale * tt.baseSize) / 2;
    return tt.x - offset;
  }

  [[nodiscard]] Renderer::TextCommand drawCenterAligned() const {
    return Renderer::TextCommand{
        .x        = alignTextToCenter(*this),
        .y        = y,
        .text     = text,
        .color    = color,
        .fontSize = static_cast<u32>(baseSize * fontScale),
    };
  }
};

struct TextAnimation {
  std::function<void(ecs::ResourceBundle, ecs::Iter<Text>, float)> animate;
  float                                                            animationSpeed;
};

void sinAnimation(ecs::ResourceBundle r, ecs::Iter<Text> t, float animationSpeed) {
  auto &time   = r.global.getResource<Time>()->get();
  t->fontScale = (std::sin(time * animationSpeed)) / 4 + 0.75;
}

struct CenterText {};

struct Input {
  Input() {
    for (auto &f : frameKeysDown) {
      f = false;
    }
    for (auto &f : prevFrameKeysDown) {
      f = false;
    }
  }

  static void pollNewInputs(ecs::Resources &global) {
    auto &input = global.getResource<Input>().value().get();

    for (u32 i = 0; i < input.frameKeysDown.size(); i++) {
      input.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
    }
  }

  static void onFrameEnd(ecs::Resources &global) {
    auto &input             = global.getResource<Input>().value().get();
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

struct MenuScreen {
  struct Option {
    Text                                     text;
    std::function<void(ecs::ResourceBundle)> onChosen;
  };

  u32 x, y;

  u32                 selectedOption = 0;
  std::vector<Option> options;

  const static inline u32 optionVerticalStride = 60;

  void handleInputs(ecs::ResourceBundle r) {
    const auto &input = r.global.getResource<Input>()->get();
    if (input.wasKeyPressed(KEY_W)) {
      if (selectedOption == 0) {
        selectedOption = options.size() - 1;
      } else {
        selectedOption -= 1;
      }
    }

    if (input.wasKeyPressed(KEY_S)) {
      selectedOption = (selectedOption + 1) % options.size();
    }

    if (input.wasKeyPressed(KEY_ENTER)) {
      options[selectedOption].onChosen(r);
    }
  }

  void drawOptions(ecs::ResourceBundle r) {
    auto &renderer = r.global.getResource<Renderer>()->get();
    for (u32 i = 0, yy = y + optionVerticalStride; i < options.size(); i++, yy += optionVerticalStride) {
      auto      &opt      = options[i];
      const auto selected = i == selectedOption;

      opt.text.x     = x;
      opt.text.y     = yy;
      opt.text.color = selected ? selectedOptionColor : defaultTextColor;

      renderer.drawText(opt.text.drawCenterAligned());
    }
  }

  void update(ecs::ResourceBundle r) {
    handleInputs(r);
    drawOptions(r);
  }

  void reset() { selectedOption = 0; }
};

void renderMenuTitle(ecs::ResourceBundle r, ecs::ComponentIter<Text, TextAnimation, CenterText> iter) {
  auto &renderer = r.global.getResource<Renderer>()->get();
  for (auto &[t, ta, tc] : iter) {
    ta->animate(r, t, ta->animationSpeed);
    renderer.drawText(t->drawCenterAligned());
  }
}

} // namespace pong
