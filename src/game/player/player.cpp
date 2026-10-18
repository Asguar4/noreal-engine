#include "../player.hpp"

#include <cmath>

#include <glm/common.hpp>

namespace riz::game {
  glm::vec2 forwardOf(float angle) noexcept {
    return glm::vec2(std::sin(angle), std::cos(angle));
  }

  glm::vec2 rightOf(float angle) noexcept {
    return glm::vec2(std::cos(angle), -std::sin(angle));
  }

  Player interpolate(const Player& previous, const Player& current, float alpha) noexcept {
    return Player{glm::mix(previous.position, current.position, alpha), glm::mix(previous.angle, current.angle, alpha)};
  }
}
