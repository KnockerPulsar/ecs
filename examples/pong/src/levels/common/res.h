#pragma once 

#include "ecs/defs.h"
#include "raylib.h"

#include <string>

namespace pong {
const auto selectedOptionColor = MAGENTA;
const auto defaultTextColor    = DARKGRAY;

namespace sceneNames {
  const std::string mainMenu = "main-menu";
  const std::string mainGame = "main-game";
  const std::string gameOver = "game-over";
} // namespace sceneNames

template <typename T>
struct Wrapper {
  T value;

  Wrapper() = default;
  Wrapper(T v) : value(v) {}

  // Allow automatic conversion to wrapped value
  operator const T &() const { return value; }
  operator T &() { return value; }

  Wrapper& operator=(T v) {
    value = v;
    return *this;
  }
};

struct Time : public Wrapper<f32> {};
struct DeltaTime : public Wrapper<f32> {};
struct ScreenWidth : public Wrapper<u32> {};
struct ScreenHeight : public Wrapper<u32> {};
enum class GameResult : u8 { Won, Lost };
struct Frame: public Wrapper<std::size_t> {};
}
