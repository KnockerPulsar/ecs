#pragma once

#include "common.h"
#include "ecs.h"
#include "level.h"
#include "resources.h"

#include <functional>
#include <string>

namespace pong {

struct PlayChosen {};
enum class AIDifficulty { Easy, Medium, Hard };

struct ScreenManager {
  std::vector<MenuScreen> screens;
  uint                    currentScreen = 0;

  // Logic to transition between screens
  std::optional<std::function<void(ScreenManager &, ecs::ResourceBundle)>> handleTransitions;

  static void update(ecs::ResourceBundle r) {
    auto &self = r.level.getResource<ScreenManager>()->get();

    if (self.handleTransitions)
      (*self.handleTransitions)(self, r);
    self.screens[self.currentScreen].update(r);
  };

  static void reset(ecs::ResourceBundle r) {
    auto &self         = r.level.getResource<ScreenManager>()->get();
    self.currentScreen = 0;

    for (auto &screen : self.screens) {
      screen.reset();
    }
  }
};

void setupMainMenu(ecs::Resources &global, ecs::Level &mm) {
  const u32 sw = global.getResource<ScreenWidth>()->get();
  const u32 sh = global.getResource<ScreenHeight>()->get();

  mm.addEntity(
      Text{
          .text     = "PONG",
          .color    = MAGENTA,
          .x        = static_cast<u32>(sw / 2.),
          .y        = static_cast<u32>(sh / 4.),
          .baseSize = 80,
      },
      TextAnimation{.animate = sinAnimation, .animationSpeed = 2},
      CenterText{}
  );

  auto mainMenu = MenuScreen{
      .x = static_cast<u32>(sw / 2.),
      .y = static_cast<u32>(sh / 3.),
      .options =
          {
              {
                  .text     = Text{.text = "Play"},
                  .onChosen = [](ecs::ResourceBundle r) { r.level.addResource(pong::PlayChosen{}); },
              },
              {
                  .text     = Text{.text = "Quit"},
                  .onChosen = [](ecs::ResourceBundle r) { r.global.addResource(ecs::Quit{}); },
              },
          },
  };

  auto difficultyMenu = MenuScreen{
      .x = static_cast<u32>(sw / 2.),
      .y = static_cast<u32>(sh / 2.),
      .options =
          {
              {
                  .text = Text{.text = "Easy"},
                  .onChosen =
                      [](ecs::ResourceBundle r) {
                        r.global.addResource(AIDifficulty::Easy);
                        r.global.addResource(ecs::TransitionToScene(pong::sceneNames::mainGame));
                      },
              },
              {
                  .text = Text{.text = "Medium"},
                  .onChosen =
                      [](ecs::ResourceBundle r) {
                        r.global.addResource(AIDifficulty::Medium);
                        r.global.addResource(ecs::TransitionToScene(pong::sceneNames::mainGame));
                      },
              },
              {
                  .text = Text{.text = "Hard"},
                  .onChosen =
                      [](ecs::ResourceBundle r) {
                        r.global.addResource(AIDifficulty::Hard);
                        r.global.addResource(ecs::TransitionToScene(pong::sceneNames::mainGame));
                      },
              },
          },
  };

  mm.addResource(ScreenManager{
      .screens = {mainMenu, difficultyMenu},
      .handleTransitions =
          [](ScreenManager &self, ecs::ResourceBundle r) {
            if (auto playChosen = r.level.consumeResource<PlayChosen>()) {
              self.currentScreen = 1;
            }
          },
  });

  // Stuff that involves rendering
  mm.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Text, TextAnimation, CenterText>>(renderMenuTitle);
  mm.addPerFrameSystem<ecs::ResourceBundle>(ScreenManager::update);

  mm.addResetSystem<ecs::ResourceBundle>(ScreenManager::reset);
}
} // namespace pong
