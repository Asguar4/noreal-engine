#pragma once

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

#include <glm/geometric.hpp>

#include "containers/bsp.hpp"

inline std::vector<riz::bsp::Segment> randomSegments(std::uint32_t seed, std::size_t count, bool onGrid, float maxLength = 0.f) {
  std::mt19937 rng(seed);
  std::vector<riz::bsp::Segment> out;
  out.reserve(count);
  std::uniform_real_distribution<float> real(-1000.f, 1000.f);
  std::uniform_int_distribution<int> grid(-16, 16);
  std::uniform_int_distribution<int> coin(0, 1);
  for (std::size_t i = 0; i < count; ++i) {
    riz::bsp::Segment s;
    if (onGrid) {
      s.a = glm::vec2(static_cast<float>(grid(rng)), static_cast<float>(grid(rng)));
      s.b = glm::vec2(static_cast<float>(grid(rng)), static_cast<float>(grid(rng)));
    } else {
      s.a = glm::vec2(real(rng), real(rng));
      s.b = glm::vec2(real(rng), real(rng));
      if (maxLength > 0.f) {
        const glm::vec2 d = s.b - s.a;
        const float len = glm::length(d);
        if (len > maxLength) {
          s.b = s.a + d * (maxLength / len);
        }
      }
    }
    s.twoSided = coin(rng) == 1;
    out.push_back(s);
  }
  return out;
}
