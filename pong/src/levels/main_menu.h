#pragma once

#include "common.h"

#include <functional>
#include <string>

namespace pong {
struct PlayChosen {};

struct MainMenu {
    u32 x, y;

    u32 selectedOption = 0;
    std::array<Text, 2> options = {
      Text{ .text = "Play", .color = WHITE, .baseSize = 40 },
      Text{ .text = "Quit", .color = WHITE, .baseSize = 40 }
    };

    const static inline u32 optionVerticalStride = 60;

    void handleInputs(ecs::Resources& r) {
      const auto& input = r.getResource<Input>()->get();
      if(input.wasKeyPressed(KEY_W)) {
	selectedOption = (selectedOption + 1) % options.size();
      }

      if(input.wasKeyPressed(KEY_S)) {
	if(selectedOption == 0) {
	  selectedOption = options.size() - 1;
	} else {
	  selectedOption -= 1;
	}
      }

      if(input.wasKeyPressed(KEY_ENTER) && selectedOption == 0) {
	r.addResource(pong::PlayChosen{});
      }
    }

    void drawOptions() {
      for(u32 i = 0, yy = y + optionVerticalStride; 
	  i < options.size(); 
	  i++, yy+= optionVerticalStride
	 ) {
	auto& opt = options[i];
	const auto selected = i == selectedOption;

	opt.x = x; 
	opt.y = yy;
	opt.color = selected? RED: WHITE;
	opt.drawCenterAligned();
      }
    }

    static void update(ecs::Resources &r, ecs::ComponentIter<MainMenu> iter) {
      for(auto& [m]: iter) {
	m->handleInputs(r);
	// Drawing
	m->drawOptions();
      }
    }
};


void renderPongLogo(ecs::Resources &r, ecs::ComponentIter<Text, TextAnimation, CenterTextAnchor> iter) {
  for (auto &[t, ta, tc] : iter) {
    t->drawCenterAligned();
    ta->animate(r, t, ta->animationSpeed);
  }
}

void setupMainMenu(ecs::Level &mm) {
  mm.addResource<Time>(0.0);
  mm.addResource<Input>(Input());

  mm.addEntity(
      Text{
	  .text = "PONG",
	  .color = MAGENTA,
          .x = static_cast<u32>(GetScreenWidth() / 2),
          .y = static_cast<u32>(GetScreenHeight() / 4),
      },
      TextAnimation { .animate = sinAnimation, .animationSpeed = 2 },
      CenterTextAnchor{}
  );

  mm.addEntity(MainMenu{
      .x = static_cast<u32>(GetScreenWidth() / 2),
      .y = static_cast<u32>(GetScreenHeight() / 3),
  });

  mm.addSystem<ecs::Resources>(Input::pollNewInputs);

  // Stuff that involves rendering
  mm.addSystem<ecs::Resources>([](ecs::Resources&){ BeginDrawing(); ClearBackground(BLACK); });
  mm.addSystem<ecs::Resources, Text, TextAnimation, CenterTextAnchor>(renderPongLogo);
  mm.addSystem<ecs::Resources, MainMenu>(MainMenu::update);
  mm.addSystem<ecs::Resources>([](ecs::Resources&){ EndDrawing(); });

  mm.addSystem<ecs::Resources>(Input::onFrameEnd);
  mm.addSystem<ecs::Resources>([](ecs::Resources& r) {
      auto &time = r.getResource<Time>().value().get();
      time += GetFrameTime();
  });
}
} // namespace pong
