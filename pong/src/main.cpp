#include "ecs.h"

#include "common/common.h"
#include "common/input.h"
#include "levels/game_over.h"
#include "levels/main_game.h"
#include "levels/main_menu.h"

#include "raylib.h"

#include <string>

int main(int argc, char** argv) {

  InitWindow(800, 800, "ecs-pong");
  SetTargetFPS(GetMonitorRefreshRate(0));

  ecs::ECS ecs;

  if(argc == 3) {
    using Recorder = pong::Input::Recorder;
    auto argStr = std::string(argv[1]);

    if(argStr == "--playback") {
      ecs.addGlobalResource(Recorder(Recorder::Mode::Playback, std::string(argv[2])));
    } else if(argStr == "--record") {
      ecs.addGlobalResource(Recorder(Recorder::Mode::Recording, std::string(argv[2])));
    }   
  }


  // Resources that are shared for all levels
  {
    ecs.addGlobalResource(pong::Frame{0});
    ecs.addGlobalResource(pong::Time{0.0});
    ecs.addGlobalResource(pong::DeltaTime{1. / 60.});
    ecs.addGlobalResource(pong::ScreenWidth(GetScreenWidth()));
    ecs.addGlobalResource(pong::ScreenHeight(GetScreenHeight()));

    ecs.addGlobalResource(pong::Input{});
    ecs.addGlobalResource(pong::Renderer{});
  }

  // Systems that run on global resources before and after each frame
  {
    ecs.addGlobalResourceSystemPre(pong::Input::pollNewInputs);

    ecs.addGlobalResourceSystemPost(pong::Renderer::system);
    ecs.addGlobalResourceSystemPost(pong::Input::onFrameEnd);
    ecs.addGlobalResourceSystemPost([](ecs::Resources &global) {
      auto &dt    = global.getResource<pong::DeltaTime>()->get();
      auto &time  = global.getResource<pong::Time>()->get();
      auto &frame = global.getResource<pong::Frame>()->get();


      frame++;
      // Clamp to ease debugging 
      dt = pong::DeltaTime(std::min(GetFrameTime(), 1/60.f)); 
      time += GetFrameTime();
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
