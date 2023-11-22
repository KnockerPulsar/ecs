#include "defs.h"
#include "level.h"
#include "raylib.h"

#include <cmath>
#include <string>

namespace pong {
struct Text {
  u32 x, y;

  std::string titleText      = "PONG";
  float       fontScale      = 1.0;
  float       animationSpeed = 1.0;

  u32 baseSize = 80;
};

struct CenterTextAnchor {};

using Time = float;

void noResource(ecs::Level::MultiIterator<Text> tt) {
  for (auto &[t] : tt) {
    std::cout << t->titleText << std::endl;
  }
}

u32 horizontalTextCenter(const Text &tt) {
  const auto offset = MeasureText(tt.titleText.c_str(), tt.fontScale * tt.baseSize) / 2;
  return tt.x - offset;
}

void renderMainMenuText(ecs::Resources &r, ecs::Level::MultiIterator<Text, CenterTextAnchor> iter) {
  auto &time = r.getResource<Time>().value().get();

  BeginDrawing();
  ClearBackground(BLACK);
  for (auto &[t, tc] : iter) {
    DrawText(
        t->titleText.c_str(), horizontalTextCenter(*t), t->y, static_cast<int>(t->baseSize * t->fontScale), MAGENTA
    );
    t->fontScale = (std::sin(time * t->animationSpeed)) / 4 + 0.75;
  }
  EndDrawing();

  time += GetFrameTime();
}

void setupMainMenu(ecs::Level &mm) {
  mm.addEntity(
      Text{
          .x              = static_cast<u32>(GetScreenWidth() / 2),
          .y              = static_cast<u32>(GetScreenHeight() / 3),
          .animationSpeed = 2,
      },
      CenterTextAnchor{}
  );
  mm.addSystem<ecs::Resources, Text, CenterTextAnchor>(renderMainMenuText);
  mm.addSystem<Text>(noResource);
  mm.addResource<Time>(0.0);
}
} // namespace pong
