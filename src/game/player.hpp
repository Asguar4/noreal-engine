#pragma once

#include <glm/vec2.hpp>

#include "../engine/loop.hpp"

namespace riz::game {
  inline constexpr double kMoveSpeed = 0.6;
  inline constexpr float kMouseSensitivity = 0.002f;

  struct Player {
    glm::vec2 position{0.f, 0.f};
    float angle = 0.f;
  };

  struct PlayerCommand {
    float forwardAxis = 0.f;
    float rightAxis = 0.f;
    float mouseDeltaX = 0.f;
  };

  constexpr float moveDistance(double seconds) noexcept {
    return static_cast<float>(kMoveSpeed * seconds);
  }

  glm::vec2 forwardOf(float angle) noexcept;
  glm::vec2 rightOf(float angle) noexcept;
  PlayerCommand playerCommandFrom(const StepInput& input) noexcept;
  Player stepPlayer(const Player& player, const PlayerCommand& command, double stepSeconds) noexcept;
  Player interpolate(const Player& previous, const Player& current, float alpha) noexcept;
}
