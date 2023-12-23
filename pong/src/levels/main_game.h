#include "common.h"
#include "defs.h"
#include "ecs.h"

namespace pong {

enum class Player : u8 { One, Two };

struct Pos2D {
  f32 x, y;
};

struct Rect {
  u32   width, height;
  Color color;
};

void renderPlayers(ecs::GlobalResources &r, ecs::ComponentIter<Pos2D, Rect> iter) {
  auto &renderer = r.getResource<Renderer>()->get();
  for (const auto &[p, r] : iter) {
    renderer.rectCommands.push_back(Renderer::RenderRectCommand{
        static_cast<u32>(p->x),
        static_cast<u32>(p->y),
        r->width,
        r->height,
        r->color,
    });
  }
};

void movePlayers(ecs::GlobalResources &r, ecs::ComponentIter<Player, Pos2D> iter) {
  const auto &inputs        = r.getResource<Input>()->get();
  const auto  dt            = r.getResource<DeltaTime>()->get();
  const auto  movementSpeed = 200.0f;

  for (auto &[pl, pos] : iter) {
    std::cout << static_cast<u8>(*pl) + 1 << "( " << pos->x << ',' << pos->y << ")\n";
    std::cout << dt << '\n';
    switch (*pl) {
    case Player::One: {

      if (inputs.isKeyDown(KEY_W)) {
        pos->y -= dt * movementSpeed;
      }

      if (inputs.isKeyDown(KEY_S)) {
        pos->y += dt * movementSpeed;
      }

      break;
    }
    case Player::Two: {

      if (inputs.isKeyDown(KEY_UP)) {
        pos->y -= dt * movementSpeed;
      }

      if (inputs.isKeyDown(KEY_DOWN)) {
        pos->y += dt * movementSpeed;
      }

      break;
    }
    }
  }
}

void setupMainGame(ecs::Level &mg) {
  const u32 sw = GetScreenWidth();
  const u32 sh = GetScreenHeight();

  const auto paddleWidth  = 20;
  const auto paddleHeight = 100;
  const auto paddleOffset = paddleWidth;

  mg.addEntity(
      Player::One,
      Pos2D{
          paddleOffset,
          static_cast<f32>((sh - paddleHeight) / 2.),
      },
      Rect{paddleWidth, paddleHeight, RED}
  );

  mg.addEntity(
      Player::Two,
      Pos2D{
          static_cast<f32>(sw - 2. * paddleOffset),
          static_cast<f32>((sh - paddleHeight) / 2.),
      },
      Rect{paddleWidth, paddleHeight, BLUE}
  );

  // Stuff that involves rendering
  mg.addSystem<ecs::GlobalResources, Pos2D, Rect>(renderPlayers);
  mg.addSystem<ecs::GlobalResources, Player, Pos2D>(movePlayers);
}

}; // namespace pong
