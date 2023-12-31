#pragma once
#include "ecs.h"
#include "level.h"
#include "resources.h"

#include "common/common.h"

#include <functional>
#include <string>

namespace pong {

struct PlayChosen {};
struct ControlsChosen {};
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
      TextAnimation{.animate = sinAnimation, .animationSpeed = 200},
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
                  .text     = Text{.text = "Controls"},
                  .onChosen = [](ecs::ResourceBundle r) { r.level.addResource(pong::ControlsChosen{}); },
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

  auto controlsScreen = MenuScreen{
      .x = static_cast<u32>(sw / 2.),
      .y = static_cast<u32>(sh / 2.33),
      .options =
          {
              {
                  .text     = Text{.text = "Back"},
                  .onChosen = [](ecs::ResourceBundle r
                              ) { r.global.addResource(ecs::TransitionToScene(pong::sceneNames::mainMenu)); },
              },
          },

      .staticContent =
          {
              {
                  Text{
                      .text     = "W/S or Up/Down arrows to move paddle/menu option",
                      .color    = WHITE,
                      .baseSize = 25,
                  },
                  Text{
                      .text     = "Enter to chose menu option, Escape to open pause menu",
                      .color    = WHITE,
                      .baseSize = 25,
                  },
              },
          }
  };

  mm.addResource(ScreenManager{
      .screens = {mainMenu, difficultyMenu, controlsScreen},
      .handleTransitions =
          [](ScreenManager &self, ecs::ResourceBundle r) {
            if (auto playChosen = r.level.consumeResource<PlayChosen>()) {
              self.currentScreen = 1;
            }

            if (auto controlsChosen = r.level.consumeResource<ControlsChosen>()) {
              self.currentScreen = 2;
            }
          },
  });

  // Stuff that involves rendering
  mm.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Text, TextAnimation, CenterText>>(renderMenuTitle);
  mm.addPerFrameSystem<ecs::ResourceBundle>(ScreenManager::update);

  mm.addResetSystem<ecs::ResourceBundle>(ScreenManager::reset);
}
} // namespace pong
