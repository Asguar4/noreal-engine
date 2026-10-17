#include <cmath>

#include "./iBsp.hpp"

namespace riz::bsp {
  double signedDistance(glm::dvec2 a, glm::dvec2 b, glm::dvec2 point) noexcept {
    const double dx = b.x - a.x;
    const double dy = b.y - a.y;
    const double cross = dx * (point.y - a.y) - dy * (point.x - a.x);
    return -cross / std::sqrt(dx * dx + dy * dy);
  }

  SegClass classifySegment(glm::dvec2 a, glm::dvec2 b, glm::dvec2 segA, glm::dvec2 segB) noexcept {
    SegClass result;
    result.distA = signedDistance(a, b, segA);
    result.distB = signedDistance(a, b, segB);
    const double dA = result.distA;
    const double dB = result.distB;
    if (std::abs(dA) <= kEpsilon && std::abs(dB) <= kEpsilon) {
      const double dot = (segB.x - segA.x) * (b.x - a.x) + (segB.y - segA.y) * (b.y - a.y);
      result.side = dot > 0.0 ? SegSide::Front : SegSide::Back;
    } else if (dA >= -kEpsilon && dB >= -kEpsilon) {
      result.side = SegSide::Front;
    } else if (dA <= kEpsilon && dB <= kEpsilon) {
      result.side = SegSide::Back;
    } else {
      result.side = SegSide::Split;
    }
    return result;
  }

  double intersectParam(glm::dvec2 a, glm::dvec2 b, glm::dvec2 l1, glm::dvec2 l2) noexcept {
    const double d1 = signedDistance(a, b, l1);
    const double d2 = signedDistance(a, b, l2);
    return d1 / (d1 - d2);
  }

  BBox bboxOf(const std::vector<glm::vec2>& points) noexcept {
    BBox box;
    if (points.empty()) {
      return box;
    }
    box.min = points.front();
    box.max = points.front();
    for (const glm::vec2& p : points) {
      bboxExpand(box, p);
    }
    return box;
  }

  void bboxExpand(BBox& box, glm::vec2 point) noexcept {
    if (point.x < box.min.x) box.min.x = point.x;
    if (point.y < box.min.y) box.min.y = point.y;
    if (point.x > box.max.x) box.max.x = point.x;
    if (point.y > box.max.y) box.max.y = point.y;
  }

  bool bboxContains(const BBox& box, glm::vec2 point) noexcept {
    return point.x >= box.min.x && point.x <= box.max.x && point.y >= box.min.y && point.y <= box.max.y;
  }
}
