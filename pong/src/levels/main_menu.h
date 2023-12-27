#pragma once

#include "common.h"
#include "ecs.h"
#include "level.h"
#include "resources.h"

#include <functional>
#include <string>

namespace pong {
struct PlayChosen {};

struct MenuOption {
  Text                                     text;
  std::function<void(ecs::ResourceBundle)> onChosen;
};

struct MenuScreen {
  u32 x, y;

  u32                     selectedOption = 0;
  std::vector<MenuOption> options;

  const static inline u32 optionVerticalStride = 60;

  void handleInputs(ecs::ResourceBundle r) {
    const auto &input = r.global.getResource<Input>()->get();
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
      opt.text.color = selected ? RED : WHITE;

      renderer.drawText(opt.text.drawCenterAligned());
    }
  }

  static void update(ecs::ResourceBundle r, ecs::ComponentIter<MenuScreen> iter) {
    for (auto &[m] : iter) {
      m->handleInputs(r);
      m->drawOptions(r);
    }
  }
};

void renderPongLogo(ecs::ResourceBundle r, ecs::ComponentIter<Text, TextAnimation, CenterTextAnchor> iter) {
  auto &renderer = r.global.getResource<Renderer>()->get();
  for (auto &[t, ta, tc] : iter) {
    ta->animate(r, t, ta->animationSpeed);
    renderer.drawText(t->drawCenterAligned());
  }
}

void setupMainMenu(ecs::Resources &global, ecs::Level &mm) {
  const u32 sw = global.getResource<ScreenWidth>()->get();
  const u32 sh = global.getResource<ScreenHeight>()->get();

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

  mm.addEntity(MenuScreen{
      .x       = static_cast<u32>(sw / 2.),
      .y       = static_cast<u32>(sh / 3.),
      .options = {
          MenuOption{
              .text     = Text{.text = "Play", .color = WHITE, .baseSize = 40},
              .onChosen = [](ecs::ResourceBundle r) { r.global.addResource(pong::PlayChosen{}); }},
          MenuOption{
              .text     = Text{.text = "Quit", .color = WHITE, .baseSize = 40},
              .onChosen = [](ecs::ResourceBundle r) { r.global.addResource(ecs::Quit{}); },
          },
      }});

  // Stuff that involves rendering
  mm.addSystem<ecs::ResourceBundle, ecs::Query<Text, TextAnimation, CenterTextAnchor>>(renderPongLogo);
  mm.addSystem<ecs::ResourceBundle, ecs::Query<MenuScreen>>(MenuScreen::update);
}
} // namespace pong
