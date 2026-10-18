#include "../level.hpp"

namespace riz::game {
  std::vector<bsp::Segment> testLevelWalls() {
    std::vector<bsp::Segment> walls;
    walls.reserve(2);
    walls.push_back(bsp::Segment{glm::vec2(-4.f, 4.f), glm::vec2(4.f, 4.f), false});
    walls.push_back(bsp::Segment{glm::vec2(2.f, 1.f), glm::vec2(2.f, 3.f), false});
    return walls;
  }

  Level buildTestLevel() {
    return Level{bsp::build(testLevelWalls()), glm::vec2(0.f, 0.f), 0.f};
  }
}
