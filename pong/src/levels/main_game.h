#include "common.h"
#include "defs.h"
#include "ecs.h"
#include "raylib.h"

namespace pong {

enum class Player : u8 { One, Two };

struct Pos2D {
  f32 x, y;
};

struct Rect {
  u32 width, height;
};

struct Circle {
  u32 radius;
};

const auto paddleWidth  = 20;
const auto paddleHeight = 100;
const auto paddleOffset = paddleWidth;

void renderPlayers(ecs::GlobalResources &r, ecs::ComponentIter<Pos2D, Rect, Color> pIter, ecs::ComponentIter<Pos2D, Circle, Color> bIter) {
  auto &renderer = r.getResource<Renderer>()->get();
  for (const auto &[p, r, c] : pIter) {
    renderer.rectCommands.push_back(Renderer::RenderRectCommand{
        static_cast<u32>(p->x),
        static_cast<u32>(p->y),
        r->width,
        r->height,
        *c,
    });
  }

  for (const auto& [p, s, c] : bIter) {
    renderer.circleCommands.push_back(Renderer::RenderCircleCommand {
        static_cast<u32>(p->x), static_cast<u32>(p->y), s->radius, *c
    });
  }
};

void movePlayers(ecs::GlobalResources &r, ecs::ComponentIter<Player, Pos2D> iter) {
  const auto &inputs        = r.getResource<Input>()->get();
  const auto  dt            = r.getResource<DeltaTime>()->get();
  const auto  movementSpeed = 200.0f;
  const auto  sh            = r.getResource<ScreenWidth>()->get();

  for (auto &[pl, pos] : iter) {
    switch (*pl) {
    case Player::One: {

      if (inputs.isKeyDown(KEY_W) && pos->y > 0) {
        pos->y -= dt * movementSpeed;
      }

      if (inputs.isKeyDown(KEY_S) && pos->y < (sh - paddleHeight)) {
        pos->y += dt * movementSpeed;
      }

      break;
    }
    case Player::Two: {

      if (inputs.isKeyDown(KEY_UP) && pos->y > 0) {
        pos->y -= dt * movementSpeed;
      }

      if (inputs.isKeyDown(KEY_DOWN) && pos->y < (sh - paddleHeight)) {
        pos->y += dt * movementSpeed;
      }

      break;
    }
    }
  }
}

void setupMainGame(ecs::GlobalResources &r, ecs::Level &mg) {
  const u32 sw = r.getResource<ScreenWidth>()->get();
  const u32 sh = r.getResource<ScreenHeight>()->get();

  mg.addEntity(
      Player::One,
      Pos2D{
          paddleOffset,
          static_cast<f32>((sh - paddleHeight) / 2.),
      },
      Rect{paddleWidth, paddleHeight},
      RED
  );

  mg.addEntity(
      Player::Two,
      Pos2D{
          static_cast<f32>(sw - 2. * paddleOffset),
          static_cast<f32>((sh - paddleHeight) / 2.),
      },
      Rect{paddleWidth, paddleHeight},
      BLUE
  );

  mg.addEntity(
      Pos2D { static_cast<f32>(sw/2.), static_cast<f32>(sh/2.) },
      Circle { 30 },
      WHITE
  );

  // Stuff that involves rendering
  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Rect, Color>, ecs::Query<Pos2D, Circle, Color>>(renderPlayers);
  mg.addSystem<ecs::GlobalResources, ecs::Query<Player, Pos2D>>(movePlayers);
}

}; // namespace pong
