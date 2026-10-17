#include <gtest/gtest.h>

#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>

#include "engine/entity.hpp"

using riz::Vector;

namespace {
constexpr float kEps = 1e-5f;
}

TEST(Camera, BasicProjection) {
  riz::Camera cam(glm::radians(90.f));
  Vector<glm::vec3> w{{3, 5, 4}, {-3, 5, 4}};
  auto r = cam.worldToScreen(w);
  ASSERT_EQ(r.size(), 4u);
  const glm::vec2 e[4] = {{0.6f, 0.8f}, {0.6f, -0.8f}, {-0.6f, 0.8f}, {-0.6f, -0.8f}};
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_NEAR(r[i].x, e[i].x, kEps) << i;
    EXPECT_NEAR(r[i].y, e[i].y, kEps) << i;
  }
}

TEST(Camera, RotatedDepth) {
  riz::Camera cam(glm::radians(90.f));
  cam.setAngle(glm::half_pi<float>());
  Vector<glm::vec3> w{{5, 1, 1}, {5, -1, 1}};
  auto r = cam.worldToScreen(w);
  ASSERT_EQ(r.size(), 4u);
  for (size_t i = 0; i < 4; ++i) EXPECT_NEAR(std::fabs(r[i].y), 0.2f, kEps);
}

TEST(Camera, BothBehind) {
  riz::Camera cam(glm::radians(90.f));
  Vector<glm::vec3> w{{0, -1, 1}, {1, -2, 1}};
  EXPECT_EQ(cam.worldToScreen(w).size(), 0u);
}

TEST(Camera, OneBehindClipped) {
  riz::Camera cam(glm::radians(90.f));
  Vector<glm::vec3> w{{0, -1, 2}, {0, 1, 2}};
  auto r = cam.worldToScreen(w);
  ASSERT_EQ(r.size(), 4u);
  for (size_t i = 0; i < 4; ++i) {
    EXPECT_TRUE(std::isfinite(r[i].x));
    EXPECT_TRUE(std::isfinite(r[i].y));
  }
  EXPECT_NEAR(std::fabs(r[0].y), 40.f, 1e-3f);
}

TEST(Camera, EndpointAtCamera) {
  riz::Camera cam(glm::radians(90.f));
  Vector<glm::vec3> w{{0, 0, 2}, {0, 3, 2}};
  auto r = cam.worldToScreen(w);
  for (size_t i = 0; i < r.size(); ++i) {
    EXPECT_TRUE(std::isfinite(r[i].x));
    EXPECT_TRUE(std::isfinite(r[i].y));
  }
}

TEST(Camera, OddCount) {
  riz::Camera cam(glm::radians(90.f));
  Vector<glm::vec3> w{{3, 5, 4}, {-3, 5, 4}, {1, 2, 3}};
  EXPECT_EQ(cam.worldToScreen(w).size(), 4u);
}
