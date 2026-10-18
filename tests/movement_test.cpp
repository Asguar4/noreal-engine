#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

#include "engine/movement.hpp"

static_assert(noexcept(riz::moveDirection(0.f, 0.f, glm::vec2{}, glm::vec2{})));

namespace {
std::vector<float> angles() {
  std::vector<float> a;
  a.reserve(3604);
  for (int k = 0; k < 3600; ++k) a.push_back(static_cast<float>(k) * 0.00174533f);
  a.push_back(-0.f);
  a.push_back(-1.f);
  a.push_back(100.f);
  a.push_back(10000.f);
  return a;
}

struct Basis {
  glm::vec2 f;
  glm::vec2 r;
};

Basis basisAt(float angle) {
  return Basis{glm::vec2(std::sin(angle), std::cos(angle)), glm::vec2(std::cos(angle), -std::sin(angle))};
}

bool bitEqual(const glm::vec2& x, const glm::vec2& y) {
  return std::memcmp(&x, &y, sizeof(glm::vec2)) == 0;
}

const float kDiagonals[4][2] = {{1.f, 1.f}, {1.f, -1.f}, {-1.f, 1.f}, {-1.f, -1.f}};
}

TEST(Movement, ZeroAxesGiveZeroVector) {
  for (float a : angles()) {
    const Basis b = basisAt(a);
    const float rights[2] = {0.f, -0.f};
    for (float mr : rights) {
      const glm::vec2 m = riz::moveDirection(0.f, mr, b.f, b.r);
      EXPECT_TRUE(m.x == 0.f && m.y == 0.f) << a;
      EXPECT_TRUE(std::isfinite(m.x) && std::isfinite(m.y)) << a;
    }
  }
}

TEST(Movement, SingleAxisBitIdenticalToLegacy) {
  const float axes[4][2] = {{1.f, 0.f}, {-1.f, 0.f}, {0.f, 1.f}, {0.f, -1.f}};
  for (float a : angles()) {
    const Basis b = basisAt(a);
    for (const auto& ax : axes) {
      const glm::vec2 got = riz::moveDirection(ax[0], ax[1], b.f, b.r);
      const glm::vec2 legacy = ax[0] * b.f + ax[1] * b.r;
      EXPECT_TRUE(bitEqual(got, legacy)) << a;
      EXPECT_TRUE(bitEqual(got * 0.005f, legacy * 0.005f)) << a;
    }
    EXPECT_TRUE(riz::moveDirection(1.f, 0.f, b.f, b.r) == b.f) << a;
    EXPECT_TRUE(riz::moveDirection(0.f, 1.f, b.f, b.r) == b.r) << a;
  }
}

TEST(Movement, DiagonalSameSpeedAsStraight) {
  for (float a : angles()) {
    const Basis b = basisAt(a);
    const float straight = glm::length(riz::moveDirection(1.f, 0.f, b.f, b.r));
    for (const auto& d : kDiagonals) {
      const glm::vec2 m = riz::moveDirection(d[0], d[1], b.f, b.r);
      EXPECT_LE(std::abs(glm::length(m) - 1.f), 1e-6f) << a;
      EXPECT_LE(std::abs(glm::length(m) - straight), 1e-6f) << a;
      EXPECT_LE(std::abs(glm::length(d[0] * b.f + d[1] * b.r) - 1.41421354f), 1e-6f) << a;
    }
  }
}

TEST(Movement, DiagonalDirectionUnchanged) {
  for (float a : angles()) {
    const Basis b = basisAt(a);
    for (const auto& d : kDiagonals) {
      const glm::vec2 m = riz::moveDirection(d[0], d[1], b.f, b.r);
      const glm::vec2 legacy = d[0] * b.f + d[1] * b.r;
      EXPECT_GE(glm::dot(glm::normalize(m), glm::normalize(legacy)), 1.f - 1e-6f) << a;
    }
  }
}

TEST(Movement, AnalogInsideUnitDiscUnchanged) {
  const float axes[3][2] = {{0.5f, 0.5f}, {0.25f, -0.75f}, {-0.5f, 0.f}};
  for (float a : angles()) {
    const Basis b = basisAt(a);
    for (const auto& ax : axes) {
      const glm::vec2 got = riz::moveDirection(ax[0], ax[1], b.f, b.r);
      EXPECT_TRUE(bitEqual(got, ax[0] * b.f + ax[1] * b.r)) << a;
    }
  }
}

TEST(Movement, OverlongAxesClampedToUnit) {
  for (float a : angles()) {
    const Basis b = basisAt(a);
    EXPECT_TRUE(bitEqual(riz::moveDirection(2.f, 0.f, b.f, b.r), riz::moveDirection(1.f, 0.f, b.f, b.r))) << a;
    const glm::vec2 m = riz::moveDirection(3.f, 4.f, b.f, b.r);
    EXPECT_LE(std::abs(glm::length(m) - 1.f), 1e-6f) << a;
    EXPECT_GE(glm::dot(glm::normalize(m), glm::normalize(3.f * b.f + 4.f * b.r)), 1.f - 1e-6f) << a;
  }
}
