#pragma once

#include <glm/vec2.hpp>

namespace riz {
  glm::vec2 moveDirection(float forwardAxis, float rightAxis, glm::vec2 forward, glm::vec2 right) noexcept;
}
