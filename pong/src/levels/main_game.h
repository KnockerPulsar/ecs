#pragma once

#include "ecs.h"
#include "level.h"
#include "resources.h"

#include "common/common.h"
#include "levels/main_menu.h"

#include "raylib.h"
#include "raymath.h"

namespace pong {
const auto ballSpeed           = 800;
const auto paddleWidth         = 20;
const auto paddleHeight        = 100;
const auto paddleOffset        = paddleWidth;
const auto playerMovementSpeed = 1200.0f;
const auto maxGoals            = 10;
const auto scoreY              = 20;

enum class Player : u8 { Human, AI };

struct Paused : Wrapper<bool> {};
struct Round : Wrapper<uint> {};
struct TimeScale : Wrapper<float> {};

struct PlayerScored {
  Player scoringPlayer;
  bool   operator==(const PlayerScored &rhs) const = default;

  struct Hasher {
    std::size_t operator()(const pong::PlayerScored &be) const noexcept {
      return static_cast<std::size_t>(be.scoringPlayer);
    }
  };
};

struct PauseScreen : MenuScreen {
  void update(ecs::ResourceBundle r) {
    this->MenuScreen::update(r);

    auto      &renderer = r.global.getResource<Renderer>()->get();
    const auto sw       = r.global.getResource<ScreenWidth>()->get();
    const auto sh       = r.global.getResource<ScreenHeight>()->get();

    renderer.drawRect(0, 0, sw, sh, ColorAlpha(BLACK, 0.7));
  }
};

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

  PlayerScore(Text &&t) : text(std::move(t)) {
    text.baseSize = 80;
    updateText();
  }

  PlayerScore() { updateText(); }

  void updateText() { text.text = std::to_string(score); }

  void incrementScore() {
    score += 1;
    updateText();
  }
};

f32 sign(float x) { return x > 0 ? 1 : -1; }
f32 frac(float x) { return x - static_cast<int>(x); }

void render(
    ecs::ResourceBundle                      r,
    ecs::ComponentIter<Pos2D, Rect, Color>   pIter,
    ecs::ComponentIter<Pos2D, Circle, Color> bIter,
    ecs::ComponentIter<PlayerScore>          sIter
) {
  auto &renderer = r.global.getResource<Renderer>()->get();

  for (const auto &[p, r, c] : pIter) {
    renderer.drawRect(p->x, p->y, r->width, r->height, *c);
  }

  for (const auto &[p, s, c] : bIter) {
    renderer.drawCircle(p->x, p->y, s->radius, *c);
  }

  for (auto &[ps] : sIter) {
    renderer.drawText(ps->text.drawCenterAligned());
  }

  const auto paused = r.level.getResource<Paused>()->get();
  if (paused) {
    auto &pauseMenu = r.level.getResource<PauseScreen>()->get();
    pauseMenu.update(r);
  }
};

void moveAI(ecs::ResourceBundle r, Pos2D &pos, Pos2D &ballPosition, Velocity &vel) {
  const auto time   = r.global.getResource<Time>()->get();
  const auto aiDiff = r.global.getResource<AIDifficulty>()->get();

  auto slowness = 0.2;
  auto speed    = 0.75;

  switch (aiDiff) {
  case AIDifficulty::Easy: {
    slowness = 0.6;
    speed    = 0.5;
    break;
  }
  case AIDifficulty::Medium: {
    slowness = 0.3;
    speed    = 0.75;
    break;
  }
  case AIDifficulty::Hard: {
    slowness = 0.1;
    speed    = 1;
    break;
  }
  }

  if (frac(time) < slowness)
    return;

  const auto playerCenter = pos.y + paddleHeight / 2;
  const auto dy = (playerCenter - ballPosition.y) / paddleHeight;

  if(std::fabs(dy) < 0.45) 
    return;

  const auto signDy       = sign(dy);

  vel.y = -playerMovementSpeed * signDy * speed;
}

void handleInputs(
    ecs::ResourceBundle r, ecs::ComponentIter<Player, Pos2D, Velocity> iter, ecs::ComponentIter<Circle, Pos2D> ball
) {

  const auto &inputs = r.global.getResource<Input>()->get();
  const auto  sh     = r.global.getResource<ScreenWidth>()->get();

  auto [_, ballPosition] = *ball.begin();

  for (auto &[pl, pos, vel] : iter) {
    if(std::fabs(vel->y) > 0) {
      vel->y *= 0.90;
    }

    switch (*pl) {
    case Player::Human: {

      if (inputs.isKeyDown(KEY_W) && pos->y > 0) {
        vel->y = -playerMovementSpeed;
      }

      if (inputs.isKeyDown(KEY_S) && pos->y < (sh - paddleHeight)) {
        vel->y = playerMovementSpeed;
      }

      break;
    }
    case Player::AI: {
      moveAI(r, *pos, *ballPosition, *vel);
      break;
    }
    }
  }

  if (inputs.wasKeyPressed(KEY_SPACE)) {
    auto &ts = r.level.getResource<TimeScale>()->get();
    if (ts == 0.1f)
      ts = TimeScale(1.0f);
    else
      ts = TimeScale(0.1f);
  }

  if (inputs.wasKeyPressed(KEY_ESCAPE)) {
    auto &paused = r.level.getResource<Paused>()->get();
    paused       = Paused(!paused);
  }
}

void moveObjects(ecs::ResourceBundle r, ecs::ComponentIter<Pos2D, Velocity> iter) {
  const auto paused = r.level.getResource<Paused>()->get();

  if (paused) {
    return;
  }

  const auto dt = r.global.getResource<DeltaTime>()->get();
  const auto ts = r.level.getResource<TimeScale>()->get();

  for (auto &[pos, vel] : iter) {
    pos->x += vel->x * dt * ts;
    pos->y += vel->y * dt * ts;
  }
}

void checkGoal(ecs::ResourceBundle r, ecs::ComponentIter<Circle, Pos2D, Velocity> iter) {
  const auto sw = r.global.getResource<ScreenWidth>()->get();
  const auto sh = r.global.getResource<ScreenHeight>()->get();

  auto &round = r.level.getResource<Round>()->get();

  for (auto &[c, p, v] : iter) {
    std::optional<Player> scoring = std::nullopt;

    const auto rightGoal = p->x > (sw - c->radius);
    const auto leftGoal  = p->x < c->radius;

    if (rightGoal) {
      scoring = Player::AI;
    } else if (leftGoal) {
      scoring = Player::Human;
    }

    if (scoring) {
      r.level.addResource(PlayerScored{*scoring});
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

void onGoal(ecs::ResourceBundle r, ecs::ComponentIter<Pos2D, Player, PlayerScore> iter) {
  auto be = r.level.consumeResource<PlayerScored>();

  // No player scored yet
  if (!be) {
    return;
  }

  for (auto &[pp, p, ps] : iter) {
    if (be->scoringPlayer == *p) {
      ps->incrementScore();
    }

    if (ps->score == maxGoals) {

      switch (*p) {
      case Player::Human:
        r.global.addResource(GameResult::Won);
        break;
      case Player::AI:
        r.global.addResource(GameResult::Lost);
        break;
      }

      r.global.addResource(ecs::TransitionToScene(pong::sceneNames::gameOver));
    }
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
    ecs::ResourceBundle                         r,
    ecs::ComponentIter<Circle, Pos2D, Velocity> balls,
    ecs::ComponentIter<Rect, Pos2D, Player>     players
) {
  const auto dt = r.global.getResource<DeltaTime>()->get();
  const auto sh = r.global.getResource<ScreenHeight>()->get();

  // Note: The Y axis for raylib is top-bottom, the x axis is left-right
  for (const auto &[circle, ballPosition, ballVelocity] : balls) {
    playerCollisions(players, *ballPosition, *ballVelocity, *circle);
    wallCollisions(*ballPosition, *ballVelocity, *circle, dt, sh);
  }
}

void resetGame(
    ecs::ResourceBundle                                      r,
    ecs::ComponentIter<Player, Pos2D, PlayerScore, Velocity> players,
    ecs::ComponentIter<Circle, Pos2D, Velocity>              ball
) {
  const f32 sw = r.global.getResource<ScreenWidth>()->get();
  const f32 sh = r.global.getResource<ScreenHeight>()->get();

  for (auto &[player, playerPos, playerScore, playerVel] : players) {
    playerPos->y = (sh - paddleHeight) / 2.f;
    *playerVel   = {0, 0};

    playerScore->score = 0;
    playerScore->updateText();
  }

  auto [_, ballPos, ballVel] = *ball.begin();

  *ballPos = {sw / 2.0f, sh / 2.0f};
  *ballVel = Velocity{ballSpeed, 0};
}

void setupMainGame(ecs::Resources global, ecs::Level &mg) {

  const f32 sw = global.getResource<ScreenWidth>()->get();
  const f32 sh = global.getResource<ScreenHeight>()->get();

  // Entity initialization
  {
    mg.addEntity(
        Player::Human,
        Pos2D{
            sw - 2.f * paddleOffset,
            (sh - paddleHeight) / 2.f,
        },
        Rect{paddleWidth, paddleHeight},
        BLUE,
        Velocity{},
        PlayerScore{Text{
            .color = BLUE,
            .x     = static_cast<u32>(sw * 2. / 3.),
            .y     = scoreY,
        }}
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
        PlayerScore{Text{
            .color = RED,
            .x     = static_cast<u32>(sw * 1. / 3.),
            .y     = scoreY,
        }}
    );

    mg.addEntity(Pos2D{sw / 2.f, sh / 2.f}, Circle{10}, Velocity{ballSpeed, 0}, WHITE);
  }

  // Level resource initialization
  {
    mg.addResource(Paused{false});
    mg.addResource(Round{0});
    mg.addResource(TimeScale{1.0f});

    // Note the extra curly brace pair since we're wrapping a MenuScreen struct
    mg.addResource(PauseScreen{{
        .x = static_cast<u32>(sw / 2.),
        .y = static_cast<u32>(sh / 2.),
        .options =
            {
                {
                    .text = Text{.text = "Resume"},
                    .onChosen =
                        [](ecs::ResourceBundle r) {
                          auto &paused = r.level.getResource<Paused>()->get();
                          paused       = Paused(!paused);
                        },
                },
                {
                    .text = Text{.text = "Main Menu"},
                    .onChosen =
                        [](ecs::ResourceBundle r) {
                          auto &paused = r.level.getResource<Paused>()->get();
                          paused       = Paused(!paused);

                          // Should allow the user to either quit or replay with another difficulty
                          r.global.addResource(ecs::TransitionToScene(sceneNames::mainMenu));
                        },
                },
            },
    }});
  }

  // Per frame systems
  {
    mg.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Player, Pos2D, Velocity>, ecs::Query<Circle, Pos2D>>(
        handleInputs
    );

    mg.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Pos2D, Velocity>>(moveObjects);
    mg.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Circle, Pos2D, Velocity>, ecs::Query<Rect, Pos2D, Player>>(
        checkCollisions
    );

    // Stuff that involves rendering
    mg.addPerFrameSystem<
        ecs::ResourceBundle,
        ecs::Query<Pos2D, Rect, Color>,
        ecs::Query<Pos2D, Circle, Color>,
        ecs::Query<PlayerScore>>(render);

    mg.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Circle, Pos2D, Velocity>>(checkGoal);
    mg.addPerFrameSystem<ecs::ResourceBundle, ecs::Query<Pos2D, Player, PlayerScore>>(onGoal);
  }

  // Reset system
  {
    mg.addResetSystem<
        ecs::ResourceBundle,
        ecs::Query<Player, Pos2D, PlayerScore, Velocity>,
        ecs::Query<Circle, Pos2D, Velocity>>(resetGame);
  }
}
}; // namespace pong
