#include "../player.hpp"

#include <cmath>

#include <glm/common.hpp>

#include "../../engine/movement.hpp"

namespace riz::game {
  static_assert(moveDistance(kDefaultStepSeconds) == 0.01f, "one 1/60 s step moves exactly 0.01 world units");

  glm::vec2 forwardOf(float angle) noexcept {
    return glm::vec2(std::sin(angle), std::cos(angle));
  }

  glm::vec2 rightOf(float angle) noexcept {
    return glm::vec2(std::cos(angle), -std::sin(angle));
  }

  PlayerCommand playerCommandFrom(const StepInput& input) noexcept {
    PlayerCommand command;
    command.forwardAxis = (input.isDown(Key::W) ? 1.f : 0.f) - (input.isDown(Key::S) ? 1.f : 0.f);
    command.rightAxis = (input.isDown(Key::D) ? 1.f : 0.f) - (input.isDown(Key::A) ? 1.f : 0.f);
    command.mouseDeltaX = input.mouseDelta().x;
    return command;
  }

  Player stepPlayer(const Player& player, const PlayerCommand& command, double stepSeconds) noexcept {
    const glm::vec2 move = moveDirection(command.forwardAxis, command.rightAxis, forwardOf(player.angle), rightOf(player.angle)) *
                           moveDistance(stepSeconds);
    return Player{player.position + move, player.angle + command.mouseDeltaX * kMouseSensitivity};
  }

  Player interpolate(const Player& previous, const Player& current, float alpha) noexcept {
    return Player{glm::mix(previous.position, current.position, alpha), glm::mix(previous.angle, current.angle, alpha)};
  }
}
