#include "level.h"
#include "common.h"


namespace pong {

void renderTestText(ecs::Resources &r, ecs::ComponentIter<Text, CenterTextAnchor> iter) {
  for (auto &[t, tc] : iter) {
    t->drawCenterAligned();
  }
}
void setupMainGame(ecs::Level &mm) {
  mm.addResource(Time{0.0});
  mm.addResource(Input());

  mm.addEntity(
      Text{
	  .text = "Main Game",
	  .color = MAGENTA,
          .x = static_cast<u32>(GetScreenWidth() / 2),
          .y = static_cast<u32>(GetScreenHeight() / 4),
      },
      CenterTextAnchor{}
  );

  mm.addSystem<ecs::Resources>(Input::pollNewInputs);

  // Stuff that involves rendering
  mm.addSystem<ecs::Resources>([](ecs::Resources&){ BeginDrawing(); ClearBackground(BLACK); });
  mm.addSystem<ecs::Resources, Text, CenterTextAnchor>(renderTestText);
  mm.addSystem<ecs::Resources>([](ecs::Resources&){ EndDrawing(); });

  mm.addSystem<ecs::Resources>(Input::onFrameEnd);
  mm.addSystem<ecs::Resources>([](ecs::Resources& r) {
      auto &time = r.getResource<Time>().value().get();
      time += GetFrameTime();
  });

}
};
