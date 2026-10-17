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

namespace {
Vector<glm::vec3> sampleWalls() {
  return Vector<glm::vec3>{{3, 5, 4}, {-3, 5, 4}, {5, 1, 1}, {5, -1, 1}, {0, -1, 1}, {1, -2, 1}, {0, 5, 2}, {0, -5, 2}, {1, 1, 1}};
}
}  // namespace

TEST(Camera, OutParamMatchesReturn) {
  const Vector<glm::vec3> sets[] = {
    {{3, 5, 4}, {-3, 5, 4}},
    {{5, 1, 1}, {5, -1, 1}},
    {{0, -1, 1}, {1, -2, 1}},
    {{0, 5, 2}, {0, -5, 2}},
    {{1, 1, 1}},
    sampleWalls(),
  };
  const float angles[] = {0.f, 0.3f, glm::half_pi<float>(), 2.5f, -1.2f};
  const glm::vec2 positions[] = {{0, 0}, {1.5f, -2.f}, {0, 4.9f}, {-3, 5}};
  const float fovs[] = {30.f, 90.f};
  for (const auto& walls : sets) {
    for (float fov : fovs) {
      for (float a : angles) {
        for (const glm::vec2& p : positions) {
          riz::Camera cam(glm::radians(fov));
          cam.setAngle(a);
          cam.setPosition(p);
          auto ret = cam.worldToScreen(walls);
          Vector<glm::vec2> out;
          cam.worldToScreen(walls, out);
          ASSERT_EQ(ret.size(), out.size());
          for (size_t i = 0; i < ret.size(); ++i) {
            EXPECT_TRUE(ret[i] == out[i]) << i;
          }
        }
      }
    }
  }
}

TEST(Camera, OutParamReusesCapacity) {
  riz::Camera cam(glm::radians(90.f));
  const auto walls = sampleWalls();
  Vector<glm::vec2> out;
  cam.worldToScreen(walls, out);
  ASSERT_GT(out.size(), 0u);
  const size_t cap = out.capacity();
  const glm::vec2* addr = &out[0];
  cam.worldToScreen(walls, out);
  EXPECT_EQ(out.capacity(), cap);
  EXPECT_EQ(&out[0], addr);
}

TEST(Camera, OutParamClearsStale) {
  riz::Camera cam(glm::radians(90.f));
  Vector<glm::vec3> walls{{3, 5, 4}, {-3, 5, 4}};
  Vector<glm::vec2> out;
  out.resize(10, glm::vec2{9.f, 9.f});
  cam.worldToScreen(walls, out);
  EXPECT_EQ(out.size(), cam.worldToScreen(walls).size());
}
