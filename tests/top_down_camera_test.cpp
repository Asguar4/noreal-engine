#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "engine/topDownCamera.hpp"

namespace {
constexpr float kEps = 1e-5f;
constexpr float kRoundTripEps = 1e-4f;

void expectVec(glm::vec2 a, glm::vec2 b, float eps = kEps) {
  EXPECT_NEAR(a.x, b.x, eps);
  EXPECT_NEAR(a.y, b.y, eps);
}

const float kAngles[] = {0.f, 0.3f, glm::half_pi<float>(), 2.5f, -1.2f};
const glm::vec2 kPositions[] = {{0.f, 0.f}, {1.5f, -2.f}, {100.f, 250.f}};
const float kZooms[] = {0.5f, 1.f, 4.f};
const glm::ivec2 kViewports[] = {{800, 600}, {600, 800}};
const glm::vec2 kPoints[] = {{0.f, 0.f}, {3.f, -4.f}, {-7.5f, 2.25f}, {120.f, 240.f}, {-300.f, 200.f}};
}

TEST(TopDownCamera, Defaults) {
  riz::TopDownCamera cam(10.f);
  expectVec(cam.getPosition(), {0.f, 0.f});
  EXPECT_FLOAT_EQ(cam.getAngle(), 0.f);
  EXPECT_FLOAT_EQ(cam.getZoom(), 1.f);
  EXPECT_FLOAT_EQ(cam.getViewHeight(), 10.f);
  EXPECT_EQ(cam.getViewportSize(), glm::ivec2(1, 1));
  EXPECT_FLOAT_EQ(cam.aspectRatio(), 1.f);
  expectVec(cam.visibleHalfExtent(), {5.f, 5.f});
}

TEST(TopDownCamera, ConstructorRejectsInvalidViewHeight) {
  EXPECT_THROW(riz::TopDownCamera cam(0.f), std::invalid_argument);
  EXPECT_THROW(riz::TopDownCamera cam(-1.f), std::invalid_argument);
  EXPECT_THROW(riz::TopDownCamera cam(std::numeric_limits<float>::quiet_NaN()), std::invalid_argument);
  EXPECT_THROW(riz::TopDownCamera cam(std::numeric_limits<float>::infinity()), std::invalid_argument);
  EXPECT_NO_THROW(riz::TopDownCamera cam(1e-3f));
}

TEST(TopDownCamera, CenterMapsToOrigin) {
  for (const glm::vec2& p : kPositions) {
    for (float a : kAngles) {
      for (float z : kZooms) {
        for (const glm::ivec2& v : kViewports) {
          riz::TopDownCamera cam(10.f);
          cam.setPosition(p);
          cam.setAngle(a);
          cam.setZoom(z);
          cam.setViewportSize(v);
          expectVec(cam.worldToScreen(cam.getPosition()), {0.f, 0.f});
        }
      }
    }
  }
}

TEST(TopDownCamera, AxesAtAngleZero) {
  riz::TopDownCamera cam(2.f);
  expectVec(cam.worldToScreen(glm::vec2{1.f, 0.f}), {1.f, 0.f});
  expectVec(cam.worldToScreen(glm::vec2{0.f, 1.f}), {0.f, 1.f});
  expectVec(cam.worldToScreen(glm::vec2{-1.f, -1.f}), {-1.f, -1.f});
}

TEST(TopDownCamera, AspectRatio) {
  riz::TopDownCamera cam(10.f);
  cam.setViewportSize({800, 600});
  EXPECT_NEAR(cam.aspectRatio(), 4.f / 3.f, kEps);
  expectVec(cam.visibleHalfExtent(), {20.f / 3.f, 5.f});
  expectVec(cam.worldToScreen(glm::vec2{0.f, 5.f}), {0.f, 1.f});
  expectVec(cam.worldToScreen(glm::vec2{20.f / 3.f, 0.f}), {1.f, 0.f});
}

TEST(TopDownCamera, ResizeKeepsVerticalExtent) {
  riz::TopDownCamera cam(10.f);
  cam.setViewportSize({1600, 600});
  EXPECT_NEAR(cam.visibleHalfExtent().y, 5.f, kEps);
  EXPECT_NEAR(cam.visibleHalfExtent().x, 40.f / 3.f, kEps);
  cam.setViewportSize({400, 600});
  EXPECT_NEAR(cam.visibleHalfExtent().y, 5.f, kEps);
  EXPECT_NEAR(cam.visibleHalfExtent().x, 10.f / 3.f, kEps);
}

TEST(TopDownCamera, Zoom) {
  riz::TopDownCamera cam(10.f);
  cam.setZoom(2.f);
  expectVec(cam.visibleHalfExtent(), {2.5f, 2.5f});
  expectVec(cam.worldToScreen(glm::vec2{0.f, 2.5f}), {0.f, 1.f});
}

TEST(TopDownCamera, InvalidZoomIgnored) {
  riz::TopDownCamera cam(10.f);
  cam.setZoom(3.f);
  cam.setZoom(0.f);
  EXPECT_FLOAT_EQ(cam.getZoom(), 3.f);
  cam.setZoom(-1.f);
  EXPECT_FLOAT_EQ(cam.getZoom(), 3.f);
  cam.setZoom(std::numeric_limits<float>::quiet_NaN());
  EXPECT_FLOAT_EQ(cam.getZoom(), 3.f);
  cam.setZoom(std::numeric_limits<float>::infinity());
  EXPECT_FLOAT_EQ(cam.getZoom(), 3.f);
}

TEST(TopDownCamera, InvalidViewportIgnored) {
  riz::TopDownCamera cam(10.f);
  cam.setViewportSize({800, 600});
  cam.setViewportSize({0, 0});
  EXPECT_EQ(cam.getViewportSize(), glm::ivec2(800, 600));
  cam.setViewportSize({-1, 5});
  EXPECT_EQ(cam.getViewportSize(), glm::ivec2(800, 600));
  cam.setViewportSize({5, 0});
  EXPECT_EQ(cam.getViewportSize(), glm::ivec2(800, 600));
  EXPECT_NEAR(cam.aspectRatio(), 4.f / 3.f, kEps);
}

TEST(TopDownCamera, RotationQuarterTurn) {
  riz::TopDownCamera cam(2.f);
  cam.setAngle(glm::half_pi<float>());
  expectVec(cam.worldToScreen(glm::vec2{1.f, 0.f}), {0.f, 1.f});
  expectVec(cam.worldToScreen(glm::vec2{0.f, -1.f}), {1.f, 0.f});
  expectVec(cam.worldToScreen(glm::vec2{0.f, 1.f}), {-1.f, 0.f});
}

TEST(TopDownCamera, RotationMatchesCameraConvention) {
  const glm::vec2 positions[] = {{0.f, 0.f}, {1.5f, -2.f}};
  for (float a : kAngles) {
    for (const glm::vec2& p : positions) {
      riz::TopDownCamera top(10.f);
      top.setAngle(a);
      top.setPosition(p);
      const glm::vec2 fwd(std::sin(a), std::cos(a));
      const glm::vec2 right(std::cos(a), -std::sin(a));
      const glm::vec2 ahead = top.worldToScreen(glm::vec2(p + 3.f * fwd));
      EXPECT_NEAR(ahead.x, 0.f, kEps);
      EXPECT_GT(ahead.y, 0.f);

      const glm::vec2 a0 = p + 5.f * fwd + 1.f * right;
      const glm::vec2 b0 = p + 6.f * fwd + 1.f * right;
      EXPECT_GT(top.worldToScreen(a0).x, 0.f);
      EXPECT_GT(top.worldToScreen(b0).x, 0.f);
    }
  }
}

TEST(TopDownCamera, ScreenRoundTrip) {
  const glm::vec2 screens[] = {{0.f, 0.f}, {1.f, 1.f}, {-0.5f, 0.75f}, {0.9f, -0.3f}};
  for (const glm::vec2& pos : kPositions) {
    for (float a : kAngles) {
      for (float z : kZooms) {
        for (const glm::ivec2& v : kViewports) {
          riz::TopDownCamera cam(10.f);
          cam.setPosition(pos);
          cam.setAngle(a);
          cam.setZoom(z);
          cam.setViewportSize(v);
          for (const glm::vec2& p : kPoints) {
            expectVec(cam.screenToWorld(cam.worldToScreen(p)), p, kRoundTripEps);
          }
          for (const glm::vec2& s : screens) {
            expectVec(cam.worldToScreen(cam.screenToWorld(s)), s, kRoundTripEps);
          }
        }
      }
    }
  }
}

TEST(TopDownCamera, PixelMapping) {
  riz::TopDownCamera cam(10.f);
  cam.setViewportSize({800, 600});
  cam.setPosition({3.f, 4.f});
  cam.setAngle(0.7f);
  expectVec(cam.worldToPixel(cam.getPosition()), {400.f, 300.f});
  expectVec(cam.pixelToWorld({0.f, 0.f}), cam.screenToWorld({-1.f, 1.f}));
  expectVec(cam.pixelToWorld({800.f, 600.f}), cam.screenToWorld({1.f, -1.f}));
  const glm::vec2 pixels[] = {{0.f, 0.f}, {800.f, 600.f}, {123.f, 456.f}, {400.5f, 17.25f}};
  for (const glm::vec2& px : pixels) {
    expectVec(cam.worldToPixel(cam.pixelToWorld(px)), px, kRoundTripEps * 10.f);
  }
}

TEST(TopDownCamera, ViewProjectionMatchesWorldToScreen) {
  for (const glm::vec2& pos : kPositions) {
    for (float a : kAngles) {
      for (float z : kZooms) {
        for (const glm::ivec2& v : kViewports) {
          riz::TopDownCamera cam(10.f);
          cam.setPosition(pos);
          cam.setAngle(a);
          cam.setZoom(z);
          cam.setViewportSize(v);
          const glm::mat4 m = cam.viewProjection();
          for (const glm::vec2& p : kPoints) {
            const glm::vec4 r = m * glm::vec4(p, 0.f, 1.f);
            expectVec(glm::vec2(r.x, r.y), cam.worldToScreen(p), kRoundTripEps);
            EXPECT_NEAR(r.z, 0.f, kEps);
            EXPECT_NEAR(r.w, 1.f, kEps);
          }
        }
      }
    }
  }
}

TEST(TopDownCamera, ViewProjectionIsOrthoAtIdentity) {
  riz::TopDownCamera cam(10.f);
  cam.setZoom(2.f);
  cam.setViewportSize({800, 600});
  const glm::vec2 he(2.5f * 4.f / 3.f, 2.5f);
  const glm::mat4 expected = glm::ortho(-he.x, he.x, -he.y, he.y, -1.f, 1.f);
  const glm::mat4 m = cam.viewProjection();
  for (int c = 0; c < 4; ++c) {
    for (int r = 0; r < 4; ++r) {
      EXPECT_NEAR(m[c][r], expected[c][r], kEps) << c << "," << r;
    }
  }
}

TEST(TopDownCamera, BatchOverloads) {
  riz::TopDownCamera cam(10.f);
  cam.setViewportSize({800, 600});
  cam.setAngle(0.4f);
  cam.setPosition({1.f, 2.f});
  const std::vector<glm::vec2> pts{{0.f, 0.f}, {1.f, 2.f}, {-3.f, 4.f}, {5.f, -6.f}};

  const auto ret = cam.worldToScreen(pts);
  ASSERT_EQ(ret.size(), pts.size());
  for (size_t i = 0; i < pts.size(); ++i) {
    expectVec(ret[i], cam.worldToScreen(pts[i]), 0.f);
  }

  std::vector<glm::vec2> out(10, glm::vec2(99.f, 99.f));
  cam.worldToScreen(pts, out);
  ASSERT_EQ(out.size(), pts.size());
  for (size_t i = 0; i < pts.size(); ++i) {
    expectVec(out[i], ret[i], 0.f);
  }

  const auto cap = out.capacity();
  const auto* data = out.data();
  cam.worldToScreen(pts, out);
  EXPECT_EQ(out.capacity(), cap);
  EXPECT_EQ(out.data(), data);
  ASSERT_EQ(out.size(), pts.size());

  const std::vector<glm::vec2> empty;
  EXPECT_TRUE(cam.worldToScreen(empty).empty());
  cam.worldToScreen(empty, out);
  EXPECT_TRUE(out.empty());
}
