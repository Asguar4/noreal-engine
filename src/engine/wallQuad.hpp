#pragma once

#include <glm/vec2.hpp>

namespace riz {
  struct WallQuad {
    glm::vec2 startTop;
    glm::vec2 startBottom;
    glm::vec2 endTop;
    glm::vec2 endBottom;
  };
}
