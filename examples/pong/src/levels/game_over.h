#pragma once

#include "ecs/ecs.h"
#include "ecs/level.h"
#include "ecs/resources.h"

#include "common/common.h"
#include "levels/main_menu.h"

#include "raylib.h"

namespace pong {
void setupGameOver(ecs::Resources &global, ecs::Level &go) {
  const u32  sw = global.getResource<ScreenWidth>()->get();
  const u32  sh = global.getResource<ScreenHeight>()->get();
  const auto gr = global.consumeResource<GameResult>().value();

  go.addEntity(
      Text{
          .text  = gr == GameResult::Won ? "You Won!" : "You Lost!",
          .color = gr == GameResult::Won ? GOLD : DARKGRAY,
          .x     = static_cast<u32>(sw / 2.),
          .y     = static_cast<u32>(sh / 4.),
      },
      TextAnimation{.animate = sinAnimation, .animationSpeed = 100},
      CenterText{}
  );

  auto gameOverMenu = MenuScreen{
      .x = static_cast<u32>(sw / 2.),
      .y = static_cast<u32>(sh / 3.),
      .options =
          {
              {
                  .text = Text{.text = "Replay"},
                  .onChosen =
                      [](ecs::ResourceBundle r) {
                        // Should already have the difficulty set, only need to reload the scene
                        r.global.addResource(ecs::TransitionToScene(sceneNames::mainGame));
                      },
              },
              {
                  .text = Text{.text = "Main Menu"},
                  .onChosen =
                      [](ecs::ResourceBundle r) {
                        // Should allow the user to either quit or replay with another difficulty
                        r.global.addResource(ecs::TransitionToScene(sceneNames::mainMenu));
                      },
              },
          },
  };

  go.addResource(ScreenManager{.screens = {gameOverMenu}});
  go.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Text, TextAnimation, CenterText>>(renderMenuTitle);
  go.addPerFrameSystem<ecs::ResourceBundle>(ScreenManager::update);

  go.addResetSystem<ecs::ResourceBundle>([&go](ecs::ResourceBundle r) { go.completeReset(); });
}
} // namespace pong
