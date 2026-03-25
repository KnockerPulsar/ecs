#include "ecs/ecs.h"

#include "common/common.h"
#include "common/input.h"
#include "levels/game_over.h"
#include "levels/main_game.h"
#include "levels/main_menu.h"

#include "raylib.h"

auto parseArguments(int argc, char **argv)
  -> std::optional<std::pair<pong::Input::InputType, std::string>> {
  auto const printHelpAndExit = [=]{
      std::println(
        std::cerr, "Usage: {} [--record <path> | --playback <path>]", argv[0]
      );
      std::exit(0);
  };

  for (int i = 1 /*skip program name*/; i < argc; i++) {
    std::string_view opt{argv[i]};
    if (opt == "--record") {
      if (i + 1 >= argc) // Check if there is no next argument
        printHelpAndExit();

      return {{pong::Input::InputType::record, std::string{argv[i + 1]}}};
    }

    if (opt == "--playback") {
      if (i + 1 >= argc)
        printHelpAndExit();

      auto const recordingPath = std::string{argv[i + 1]};
      if (!std::filesystem::exists(recordingPath)) {
        std::println(
          std::cerr,
          "Error opening file '{}': File does not exist",
          recordingPath
        );
        std::exit(1);
      }

      if (!std::filesystem::is_regular_file(recordingPath)) {
        std::println(
          std::cerr, "Error opening file '{}': Is not a file", recordingPath
        );
        std::exit(1);
      }

      return {{pong::Input::InputType::playback, recordingPath}};
    }

    printHelpAndExit();
  }

    return std::nullopt;
}

int main(int argc, char **argv) {
  auto const inputTypeAndPath = parseArguments(argc, argv);

  // TODO maybe this should be moved into the renderer
  InitWindow(800, 800, "ecs-pong");
  SetTargetFPS(60);

  ecs::ECS ecs;

  // Resources that are shared for all levels
  {
    ecs.addGlobalResource(pong::Frame{0});
    ecs.addGlobalResource(pong::Time{0.0});
    ecs.addGlobalResource(pong::DeltaTime{1. / 60.});
    ecs.addGlobalResource(pong::ScreenWidth(GetScreenWidth()));
    ecs.addGlobalResource(pong::ScreenHeight(GetScreenHeight()));

    if (inputTypeAndPath) {
      auto const [inputType, recordingPath] = *inputTypeAndPath;
      ecs.addGlobalResource(pong::Input{inputType, recordingPath});
    } else {
      ecs.addGlobalResource(pong::Input{});
    }

    ecs.addGlobalResource(pong::Renderer{});
  }

  // Systems that run on global resources before and after each frame
  {
    ecs.addGlobalResourceSystemPre(pong::Input::pollNewInputs);

    ecs.addGlobalResourceSystemPost(pong::Renderer::system);
    ecs.addGlobalResourceSystemPost(pong::Input::onFrameEnd);
    ecs.addGlobalResourceSystemPost([&](ecs::Resources &global) {
      auto &dt    = global.getResource<pong::DeltaTime>()->get();
      auto &time  = global.getResource<pong::Time>()->get();
      auto &frame = global.getResource<pong::Frame>()->get();
      auto &input = global.getResource<pong::Input>()->get();

      if(inputTypeAndPath)
      {
        auto const [inputType, _] = *inputTypeAndPath;
        if (auto const currentInputState = input.getCurrentInputState()) {
          dt = std::get<0>(*currentInputState);
        }
      } else {
        dt = pong::DeltaTime{GetFrameTime()};
      }

      time += dt;
      frame += 1;
    });
  }

  // Levels and their setup code
  ecs.addStartupLevel(pong::sceneNames::mainMenu, pong::setupMainMenu);
  ecs.addLevel(pong::sceneNames::mainGame, pong::setupMainGame);
  ecs.addLevel(pong::sceneNames::gameOver, pong::setupGameOver);

  ecs.runSetupSystems();
  while (!ecs.shouldQuit()) {
    ecs.checkTransitions();

    ecs.runPreSystems();
    ecs.runPerFrameSystems();
    ecs.runPostSystems();
  }

  CloseWindow();
  return 0;
}
