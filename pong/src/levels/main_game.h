#include "common.h"
#include "defs.h"
#include "ecs.h"
#include "level.h"
#include "raylib.h"

#include <unordered_set>

namespace pong {

enum class Player : u8 { One, Two };

struct BallEvent {
  Player scoringPlayer;
  bool operator==(const BallEvent& rhs) const = default;
};

} // namespace pong

// Custom specialization of std::hash can be injected in namespace std.
template <>
struct std::hash<pong::BallEvent> {
  std::size_t operator()(const pong::BallEvent &be) const noexcept { return static_cast<std::size_t>(be.scoringPlayer); }
};

namespace pong {
struct Pos2D {
  f32 x, y;
};

struct Rect {
  u32 width, height;
};

struct Circle {
  u32 radius;
};

struct Velocity {
  f32 x, y;
};

struct PlayerScore {
  Text text;
  u32  score = 0;

  PlayerScore() = default;

  PlayerScore(Color c, u32 x, u32 y) : text({"", c}) {
    text.x    = x;
    text.y    = y;
    text.text = std::to_string(0);
  }

  void incrementScore() {
    score += 1;
    text.text = std::to_string(score);
  }
};

struct BallEvents : Wrapper<std::unordered_set<BallEvent>> {};

const auto paddleWidth  = 20;
const auto paddleHeight = 100;
const auto paddleOffset = paddleWidth;

void renderPlayers(ecs::GlobalResources &r, ecs::ComponentIter<Pos2D, Rect, Color> pIter) {
  auto &renderer = r.getResource<Renderer>()->get();
  for (const auto &[p, r, c] : pIter) {
    renderer.drawRect(p->x, p->y, r->width, r->height, *c);
  }
};

void renderBalls(ecs::GlobalResources &r, ecs::ComponentIter<Pos2D, Circle, Color> bIter) {
  auto &renderer = r.getResource<Renderer>()->get();

  for (const auto &[p, s, c] : bIter) {
    renderer.drawCircle(p->x, p->y, s->radius, *c);
  }
};

void movePlayers(ecs::GlobalResources &r, ecs::ComponentIter<Player, Pos2D, Velocity> iter) {
  const auto &inputs = r.getResource<Input>()->get();

  const auto movementSpeed = 200.0f;
  const auto sh            = r.getResource<ScreenWidth>()->get();

  for (auto &[pl, pos, vel] : iter) {
    vel->x *= (0.975);
    vel->y *= (0.975);

    switch (*pl) {
    case Player::One: {

      if (inputs.isKeyDown(KEY_W) && pos->y > 0) {
        vel->y = -movementSpeed;
      }

      if (inputs.isKeyDown(KEY_S) && pos->y < (sh - paddleHeight)) {
        vel->y = movementSpeed;
      }

      break;
    }
    case Player::Two: {

      if (inputs.isKeyDown(KEY_UP) && pos->y > 0) {
        vel->y = -movementSpeed;
      }

      if (inputs.isKeyDown(KEY_DOWN) && pos->y < (sh - paddleHeight)) {
        vel->y = movementSpeed;
      }

      break;
    }
    }
  }
}

void moveObjects(ecs::GlobalResources &r, ecs::ComponentIter<Pos2D, Velocity> iter) {
  const auto dt = r.getResource<DeltaTime>()->get();
  for (auto &[pos, vel] : iter) {
    pos->x += vel->x * dt;
    pos->y += vel->y * dt;
  }
}

void checkGoal(ecs::GlobalResources &gr, ecs::LevelResources& lr, ecs::MultiIterator<Circle, Pos2D> iter) {
  const auto sw         = gr.getResource<ScreenWidth>()->get();
  auto      &ballEvents = lr.getResource<BallEvents>()->get();

  for (auto &[c, p] : iter) {
    if (p->x > (sw - c->radius)) {
      ballEvents.value.insert(BallEvent{Player::One});
      p->x = sw / 2;
    }

    if (p->x < c->radius) {
      ballEvents.value.insert(BallEvent{Player::Two});
      p->x = sw / 2;
    }
  }
}

void updateScore(ecs::LevelResources &lr, ecs::MultiIterator<Player, PlayerScore> iter) {
  auto &ballEvents = lr.getResource<BallEvents>()->get();

  for (const auto &be : ballEvents.value) {
    for (auto &[p, ps] : iter) {
      if (be.scoringPlayer == *p) {
        std::cout << "Player " << static_cast<u8>(be.scoringPlayer) + 1 << " scores\n";
        ps->incrementScore();
      }
    }
  }

  ballEvents.value.clear();
}

void renderScores(ecs::GlobalResources &r, ecs::MultiIterator<PlayerScore> iter) {
  auto &renderer = r.getResource<Renderer>()->get();

  for (auto &[ps] : iter) {
    renderer.drawText(ps->text.drawCenterAligned());
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
      RED,
      Velocity{},
      PlayerScore{RED, static_cast<u32>(sw * 1. / 3.), 100}
  );

  mg.addEntity(
      Player::Two,
      Pos2D{
          static_cast<f32>(sw - 2. * paddleOffset),
          static_cast<f32>((sh - paddleHeight) / 2.),
      },
      Rect{paddleWidth, paddleHeight},
      BLUE,
      Velocity{},
      PlayerScore{BLUE, static_cast<u32>(sw * 2. / 3.), 100}
  );

  mg.addEntity(Pos2D{static_cast<f32>(sw / 2.), static_cast<f32>(sh / 2.)}, Circle{20}, Velocity{100, 0}, WHITE);

  mg.addResource(BallEvents{});

  // Stuff that involves rendering
  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Rect, Color>>(renderPlayers);
  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Circle, Color>>(renderBalls);
  mg.addSystem<ecs::GlobalResources, ecs::Query<PlayerScore>>(renderScores);

  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Velocity>>(moveObjects);
  mg.addSystem<ecs::GlobalResources, ecs::Query<Player, Pos2D, Velocity>>(movePlayers);

  mg.addSystem<ecs::GlobalResources, ecs::LevelResources, ecs::Query<Circle, Pos2D>>(checkGoal);
  mg.addSystem<ecs::LevelResources, ecs::Query<Player, PlayerScore>>(updateScore);
}

}; // namespace pong
