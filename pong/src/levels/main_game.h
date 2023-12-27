#include "common.h"
#include "defs.h"
#include "ecs.h"
#include "level.h"
#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <ios>
#include <unordered_set>

namespace pong {

const auto ballSpeed    = 800;
const auto paddleWidth  = 20;
const auto paddleHeight = 100;
const auto paddleOffset = paddleWidth;

enum class Player : u8 { One, AI };

struct Round : Wrapper<uint> {};
struct TimeScale : Wrapper<float> {};

struct PlayerScored {
  Player scoringPlayer;
  bool   operator==(const PlayerScored &rhs) const = default;
};

} // namespace pong

// Custom specialization of std::hash can be injected in namespace std.
template <>
struct std::hash<pong::PlayerScored> {
  std::size_t operator()(const pong::PlayerScored &be) const noexcept {
    return static_cast<std::size_t>(be.scoringPlayer);
  }
};

namespace pong {

struct Pos2D : Vector2 {};
struct Velocity : Vector2 {};

struct Rect {
  f32 width, height;
};

struct Circle {
  f32 radius;
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

struct BallEvents : Wrapper<std::unordered_set<PlayerScored>> {};

f32 sign(float x) { return x > 0 ? 1 : -1; }

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

void handleInputs(
    ecs::GlobalResources &r, ecs::ComponentIter<Player, Pos2D, Velocity> iter, ecs::ComponentIter<Circle, Pos2D> ball
) {
  const auto &inputs = r.getResource<Input>()->get();

  const auto playerMovementSpeed = 1200.0f;
  const auto sh                  = r.getResource<ScreenWidth>()->get();
  const auto time                = r.getResource<Time>()->get();
  auto      &ts                  = r.getResource<TimeScale>()->get();

  auto &[_, ballPosition] = *ball.begin();

  for (auto &[pl, pos, vel] : iter) {
    vel->x *= (0.975);
    vel->y *= (0.975);

    switch (*pl) {
    case Player::One: {

      if (inputs.isKeyDown(KEY_W) && pos->y > 0) {
        vel->y = -playerMovementSpeed;
      }

      if (inputs.isKeyDown(KEY_S) && pos->y < (sh - paddleHeight)) {
        vel->y = playerMovementSpeed;
      }

      break;
    }
    case Player::AI: {
      if (fmod(time, 1.0f) < 0.2)
        break;

      const auto playerCenter = pos->y + paddleHeight;
      const auto signDy       = -sign(playerCenter - ballPosition->y);
      vel->y                  = playerMovementSpeed * signDy;
      break;
    }
    }
  }

  std::cout << ts << "\n";
  if (inputs.wasKeyPressed(KEY_SPACE)) {
    if (ts == 0.1f)
      ts = TimeScale(1.0f);
    else
      ts = TimeScale(0.1f);
  }
}

void moveObjects(ecs::GlobalResources &r, ecs::ComponentIter<Pos2D, Velocity> iter) {
  const auto dt = r.getResource<DeltaTime>()->get();
  const auto ts = r.getResource<TimeScale>()->get();
  for (auto &[pos, vel] : iter) {
    pos->x += vel->x * dt * ts;
    pos->y += vel->y * dt * ts;
  }
}

void checkGoal(ecs::GlobalResources &gr, ecs::LevelResources &lr, ecs::ComponentIter<Circle, Pos2D, Velocity> iter) {
  const auto sw         = gr.getResource<ScreenWidth>()->get();
  const auto sh         = gr.getResource<ScreenHeight>()->get();
  auto      &ballEvents = lr.getResource<BallEvents>()->get();
  auto      &round      = lr.getResource<Round>()->get();

  for (auto &[c, p, v] : iter) {
    std::optional<Player> scoring = std::nullopt;

    const auto rightGoal = p->x > (sw - c->radius);
    const auto leftGoal  = p->x < c->radius;

    if (rightGoal) {
      scoring = Player::AI;
    } else if (leftGoal) {
      scoring = Player::One;
    }

    if (scoring) {
      ballEvents.value.insert(PlayerScored{*scoring});
      p->x = sw / 2;
      p->y = sh / 2;
      round += 1;

      // Flip the ball's direction every round
      if (round % 2 == 0) {
        *v = {ballSpeed, 0};
      } else {
        *v = {-ballSpeed, 0};
      }
    }
  }
}

void onGoal(ecs::GlobalResources &gr, ecs::LevelResources &lr, ecs::ComponentIter<Pos2D, Player, PlayerScore> iter) {
  auto &ballEvents = lr.getResource<BallEvents>()->get();

  for (const auto &be : ballEvents.value) {
    for (auto &[pp, p, ps] : iter) {
      if (be.scoringPlayer == *p) {
        ps->incrementScore();
      }
    }
  }

  ballEvents.value.clear();
}

void renderScores(ecs::GlobalResources &r, ecs::ComponentIter<PlayerScore> iter) {
  auto &renderer = r.getResource<Renderer>()->get();

  for (auto &[ps] : iter) {
    renderer.drawText(ps->text.drawCenterAligned());
  }
}

void onBallHit(Pos2D &ballPosition, Velocity &ballVelocity, Pos2D &playerPosition, Rect &rect) {
  const auto hitSpeedIncrease = 25;
  const auto dy               = ballPosition.y - playerPosition.y;
  auto       bounceAngle      = 90;

  // Going right
  if (sign(ballVelocity.x) == 1) {
    if (dy < 0) {
      bounceAngle = 135;
    } else if (dy > rect.height) {
      bounceAngle = -135;
    } else {
      auto relDy = dy / rect.height; // 0 .. 1

      if (relDy == 0.5) {
        bounceAngle = 1;
      } else {
        bounceAngle = std::lerp(105, 255, relDy);
      }
    }
  } else {
    // Going left
    if (dy < 0) {
      bounceAngle = 45;
    } else if (dy > rect.height) {
      bounceAngle = -45;
    } else {
      auto relDy = dy / rect.height; // 0 .. 1

      if (relDy == 0.5) {
        bounceAngle = 1;
      } else {
        bounceAngle = std::lerp(75, -75, relDy);
      }
    }
  }

  const auto speed = Vector2Length(ballVelocity) + hitSpeedIncrease;
  ballVelocity.x   = std::cos(DEG2RAD * bounceAngle) * speed;
  ballVelocity.y   = -std::sin(DEG2RAD * bounceAngle) * speed; // The Y axis is flipped for raylib
}

void resolveBallCollision(Pos2D &ballPosition, Velocity &ballVelocity, Circle &ballCircle, Pos2D &playerPosition) {
  auto dx = ballPosition.x - playerPosition.x;

  // Account for the pivot of the player rectangle being skewed to the left.
  if (ballVelocity.x < 0) {
    dx -= paddleWidth;
  }

  ballPosition.x -= sign(ballVelocity.x) * (dx + ballCircle.radius + 1);
}

void playerCollisions(
    ecs::ComponentIter<Rect, Pos2D, Player> players, Pos2D &ballPosition, Velocity &ballVelocity, Circle &circle
) {
  for (const auto &[rect, playerPosition, player] : players) {
    const auto playerRect =
        Rectangle{playerPosition->x, playerPosition->y, static_cast<f32>(rect->width), static_cast<f32>(rect->height)};

    const auto hit = CheckCollisionCircleRec(ballPosition, circle.radius, playerRect);

    if (hit) {
      resolveBallCollision(ballPosition, ballVelocity, circle, *playerPosition);
      onBallHit(ballPosition, ballVelocity, *playerPosition, *rect);
      break;
    }
  }
}

void wallCollisions(Pos2D &ballPosition, Velocity &ballVelocity, Circle &circle, float dt, float sh) {
  const auto testY = ballPosition.y + sign(ballVelocity.y) * circle.radius;

  const auto hitTop    = (0 >= testY);
  const auto hitBottom = (sh <= testY);

  if (hitTop || hitBottom) {
    ballVelocity.y *= -1;
  }
}

void checkCollisions(
    ecs::GlobalResources                       &r,
    ecs::ComponentIter<Circle, Pos2D, Velocity> balls,
    ecs::ComponentIter<Rect, Pos2D, Player>     players
) {
  const auto dt = r.getResource<DeltaTime>()->get();
  const auto sh = r.getResource<ScreenHeight>()->get();

  // Note: The Y axis for raylib is top-bottom, the x axis is left-right
  for (const auto &[circle, ballPosition, ballVelocity] : balls) {
    playerCollisions(players, *ballPosition, *ballVelocity, *circle);
    wallCollisions(*ballPosition, *ballVelocity, *circle, dt, sh);
  }
}

void setupMainGame(ecs::GlobalResources &r, ecs::Level &mg) {

  const f32  sw     = r.getResource<ScreenWidth>()->get();
  const f32  sh     = r.getResource<ScreenHeight>()->get();
  const auto scoreY = 20;

  r.addResource(TimeScale{1.0f});

  mg.addEntity(
      Player::One,
      Pos2D{
          sw - 2.f * paddleOffset,
          (sh - paddleHeight) / 2.f + 10,
      },
      Rect{paddleWidth, paddleHeight},
      BLUE,
      Velocity{},
      PlayerScore{BLUE, static_cast<u32>(sw * 2. / 3.), scoreY}
  );

  mg.addEntity(
      Player::AI,
      Pos2D{
          paddleOffset,
          (sh - paddleHeight) / 2.f,
      },
      Rect{paddleWidth, paddleHeight},
      RED,
      Velocity{},
      PlayerScore{RED, static_cast<u32>(sw * 1. / 3.), scoreY}
  );

  mg.addEntity(Pos2D{sw / 2.f, sh / 2.f}, Circle{10}, Velocity{ballSpeed, 0}, WHITE);

  mg.addResource(BallEvents{});
  mg.addResource(Round{0});

  // Stuff that involves rendering
  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Rect, Color>>(renderPlayers);
  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Circle, Color>>(renderBalls);
  mg.addSystem<ecs::GlobalResources, ecs::Query<PlayerScore>>(renderScores);

  mg.addSystem<ecs::GlobalResources, ecs::Query<Pos2D, Velocity>>(moveObjects);
  mg.addSystem<ecs::GlobalResources, ecs::Query<Player, Pos2D, Velocity>, ecs::Query<Circle, Pos2D>>(handleInputs);
  mg.addSystem<ecs::GlobalResources, ecs::Query<Circle, Pos2D, Velocity>, ecs::Query<Rect, Pos2D, Player>>(
      checkCollisions
  );

  mg.addSystem<ecs::GlobalResources, ecs::LevelResources, ecs::Query<Circle, Pos2D, Velocity>>(checkGoal);
  mg.addSystem<ecs::GlobalResources, ecs::LevelResources, ecs::Query<Pos2D, Player, PlayerScore>>(onGoal);
}
}; // namespace pong
