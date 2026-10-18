#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include <glm/common.hpp>
#include <glm/vec2.hpp>

#include "engine/camera.hpp"
#include "game/player.hpp"

namespace {
  bool bitEqual(glm::vec2 a, glm::vec2 b) {
    return std::memcmp(&a, &b, sizeof(glm::vec2)) == 0;
  }

  bool bitEqual(float a, float b) {
    return std::memcmp(&a, &b, sizeof(float)) == 0;
  }
}

TEST(Player, Defaults) {
  const riz::game::Player player{};
  EXPECT_EQ(player.position, glm::vec2(0.f, 0.f));
  EXPECT_EQ(player.angle, 0.f);
}

TEST(Player, BasisMatchesCamera) {
  std::vector<float> angles;
  for (int k = 0; k < 3600; ++k) angles.push_back(static_cast<float>(k) * 0.00174533f);
  angles.push_back(-1.f);
  angles.push_back(100.f);
  riz::Camera camera(0.5f);
  for (const float angle : angles) {
    camera.setAngle(angle);
    EXPECT_TRUE(bitEqual(riz::game::forwardOf(angle), camera.forward())) << angle;
    EXPECT_TRUE(bitEqual(riz::game::rightOf(angle), camera.right())) << angle;
  }
}

TEST(Player, InterpolateMatchesMix) {
  const riz::game::Player pairs[][2] = {
    {{glm::vec2(0.f, 0.f), 0.f}, {glm::vec2(1.f, 2.f), 0.5f}},
    {{glm::vec2(-3.5f, 7.25f), -1.f}, {glm::vec2(4.f, -0.125f), 3.f}},
    {{glm::vec2(0.1f, 0.2f), 100.f}, {glm::vec2(0.3f, 0.7f), 100.5f}}};
  const float alphas[] = {0.f, 0.25f, 0.5f, 1.f};
  for (const auto& pair : pairs) {
    for (const float alpha : alphas) {
      const riz::game::Player result = riz::game::interpolate(pair[0], pair[1], alpha);
      EXPECT_TRUE(bitEqual(result.position, glm::mix(pair[0].position, pair[1].position, alpha)));
      EXPECT_TRUE(bitEqual(result.angle, glm::mix(pair[0].angle, pair[1].angle, alpha)));
    }
    const riz::game::Player atZero = riz::game::interpolate(pair[0], pair[1], 0.f);
    EXPECT_EQ(atZero.position, pair[0].position);
    EXPECT_EQ(atZero.angle, pair[0].angle);
    const riz::game::Player atOne = riz::game::interpolate(pair[0], pair[1], 1.f);
    EXPECT_EQ(atOne.position, pair[1].position);
    EXPECT_EQ(atOne.angle, pair[1].angle);
  }
}
