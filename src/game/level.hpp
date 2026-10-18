#pragma once

#include <vector>

#include <glm/vec2.hpp>

#include "../containers/bsp.hpp"

namespace riz::game {
  struct Level {
    bsp::Tree tree;
    glm::vec2 playerStart{0.f, 0.f};
    float playerStartAngle = 0.f;
  };

  std::vector<bsp::Segment> testLevelWalls();
  Level buildTestLevel();
}
