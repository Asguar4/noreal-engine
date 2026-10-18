#pragma once

#include <glm/vec2.hpp>

namespace riz::game {
  struct Player {
    glm::vec2 position{0.f, 0.f};
    float angle = 0.f;
  };

  glm::vec2 forwardOf(float angle) noexcept;
  glm::vec2 rightOf(float angle) noexcept;
  Player interpolate(const Player& previous, const Player& current, float alpha) noexcept;
}
