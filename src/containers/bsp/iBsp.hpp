#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include "../bsp.hpp"

namespace riz::bsp {
  inline constexpr double kEpsilon = 1.0 / 1024.0;
  inline constexpr double kSplitCost = 8.0;

  enum class SegSide : std::uint8_t { Front, Back, Split };

  struct SegClass {
    SegSide side = SegSide::Front;
    double distA = 0.0;
    double distB = 0.0;
  };

  double signedDistance(glm::dvec2 a, glm::dvec2 b, glm::dvec2 point) noexcept;
  SegClass classifySegment(glm::dvec2 a, glm::dvec2 b, glm::dvec2 segA, glm::dvec2 segB) noexcept;
  double intersectParam(glm::dvec2 a, glm::dvec2 b, glm::dvec2 l1, glm::dvec2 l2) noexcept;

  BBox bboxOf(const std::vector<glm::vec2>& points) noexcept;
  void bboxExpand(BBox& box, glm::vec2 point) noexcept;
  bool bboxContains(const BBox& box, glm::vec2 point) noexcept;

  std::vector<std::string> checkInvariants(const Tree& tree);
}
