#include "level.h"
#include "common.h"


namespace pong {

void renderTestText(ecs::GlobalResources &r, ecs::ComponentIter<Text, CenterTextAnchor> iter) {
  auto& renderer = r.getResource<Renderer>()->get();
  for (auto &[t, tc] : iter) {
    renderer.textCommands.push_back(t->drawCenterAligned());
  }
}

void setupMainGame(ecs::Level &mm) {
  mm.addEntity(
      Text{
	  .text = "Main Game",
	  .color = MAGENTA,
          .x = static_cast<u32>(GetScreenWidth() / 2),
          .y = static_cast<u32>(GetScreenHeight() / 4),
      },
      CenterTextAnchor{}
  );

  // Stuff that involves rendering
  mm.addSystem<ecs::GlobalResources, Text, CenterTextAnchor>(renderTestText);
}

};
