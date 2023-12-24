#pragma once

#include "common.h"
#include "ecs.h"
#include "level.h"

#include <functional>
#include <string>

namespace pong {
struct PlayChosen {};

struct MainMenu {
  u32 x, y;

  u32                 selectedOption = 0;
  std::array<Text, 2> options        = {
      Text{.text = "Play", .color = WHITE, .baseSize = 40}, 
      Text{.text = "Quit", .color = WHITE, .baseSize = 40},
  };

  const static inline u32 optionVerticalStride = 60;

  void handleInputs(ecs::GlobalResources &r) {
    const auto &input = r.getResource<Input>()->get();
    if (input.wasKeyPressed(KEY_W)) {
      selectedOption = (selectedOption + 1) % options.size();
    }

    if (input.wasKeyPressed(KEY_S)) {
      if (selectedOption == 0) {
        selectedOption = options.size() - 1;
      } else {
        selectedOption -= 1;
      }
    }

    if (input.wasKeyPressed(KEY_ENTER) && selectedOption == 0) {
      r.addResource(pong::PlayChosen{});
    }
  }

  void drawOptions(ecs::GlobalResources &r) {
    auto &renderer = r.getResource<Renderer>()->get();
    for (u32 i = 0, yy = y + optionVerticalStride; i < options.size(); i++, yy += optionVerticalStride) {
      auto      &opt      = options[i];
      const auto selected = i == selectedOption;

      opt.x     = x;
      opt.y     = yy;
      opt.color = selected ? RED : WHITE;

      renderer.drawText(opt.drawCenterAligned());
    }
  }

  static void update(ecs::GlobalResources &r, ecs::ComponentIter<MainMenu> iter) {
    for (auto &[m] : iter) {
      m->handleInputs(r);
      m->drawOptions(r);
    }
  }
};

void renderPongLogo(ecs::GlobalResources &r, ecs::ComponentIter<Text, TextAnimation, CenterTextAnchor> iter) {
  auto &renderer = r.getResource<Renderer>()->get();
  for (auto &[t, ta, tc] : iter) {
    ta->animate(r, t, ta->animationSpeed);
    renderer.drawText(t->drawCenterAligned());
  }
}

void setupMainMenu(ecs::GlobalResources &r, ecs::Level &mm) {
  const u32 sw = r.getResource<ScreenWidth>()->get();
  const u32 sh = r.getResource<ScreenHeight>()->get();

  std::cerr << sw << '\n' << sh << '\n';

  mm.addEntity(
      Text{
          .text  = "PONG",
          .color = MAGENTA,
          .x     = static_cast<u32>(sw / 2.),
          .y     = static_cast<u32>(sh / 4.),
      },
      TextAnimation{.animate = sinAnimation, .animationSpeed = 2},
      CenterTextAnchor{}
  );

  mm.addEntity(MainMenu{
      .x = static_cast<u32>(sw / 2.),
      .y = static_cast<u32>(sh / 3.),
  });

  // Stuff that involves rendering
  mm.addSystem<ecs::GlobalResources, ecs::Query<Text, TextAnimation, CenterTextAnchor>>(renderPongLogo);
  mm.addSystem<ecs::GlobalResources, ecs::Query<MainMenu>>(MainMenu::update);
}
} // namespace pong
