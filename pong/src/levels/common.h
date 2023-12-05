#pragma once 

#include "defs.h"
#include "level.h"
#include "raylib.h"

#include <cmath>

namespace pong {
using Time = float;

struct Text {
  std::string text;
  Color color;

  float fontScale = 1.0;
  u32 x = 0;
  u32 y = 0;
  u32 baseSize = 80;

  static u32 alignTextToCenter(const Text &tt) {
    const auto offset = MeasureText(tt.text.c_str(), tt.fontScale * tt.baseSize) / 2;
    return tt.x - offset;
  }

  void drawCenterAligned() const {
    DrawText(text.c_str(), alignTextToCenter(*this), y, static_cast<int>(baseSize * fontScale), color);
  }
};

struct TextAnimation { 
  std::function<void(ecs::Resources&, ecs::Iter<Text>, float)> animate; 
  float animationSpeed;
};

void sinAnimation(ecs::Resources& r, ecs::Iter<Text> t, float animationSpeed) {
  auto &time = r.getResource<Time>().value().get();
  t->fontScale = (std::sin(time * animationSpeed)) / 4 + 0.75;
}

struct CenterTextAnchor { };

struct Input {
    Input() {
	for(auto& f: frameKeysDown) { f = false; }
	for(auto& f: prevFrameKeysDown) { f = false; }
    }

    static void pollNewInputs(ecs::Resources& r) {
	auto& input = r.getResource<Input>().value().get();

	for(int i = 0; i < input.frameKeysDown.size(); i++) {
	    input.frameKeysDown[i] = IsKeyDown(static_cast<KeyboardKey>(i));
	}
    }

    static void onFrameEnd(ecs::Resources& r) {
	auto& input = r.getResource<Input>().value().get();
	input.prevFrameKeysDown = input.frameKeysDown;

	for(auto& f: input.frameKeysDown) { f = false; }
    }

    bool isKeyDown(KeyboardKey k) const {
	return frameKeysDown[static_cast<u32>(k)];
    }

    bool wasKeyPressed(KeyboardKey k) const {
	return frameKeysDown[static_cast<u32>(k)] && !prevFrameKeysDown[static_cast<u32>(k)];
    }

private:
    // Raylib's `KeyboardKey` enum has a maximum value of 348.
    std::array<bool, 348> frameKeysDown, prevFrameKeysDown;
};
}
