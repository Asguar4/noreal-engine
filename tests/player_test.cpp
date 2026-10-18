#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <vector>

#include <glm/common.hpp>
#include <glm/vec2.hpp>

#include "engine/input.hpp"
#include "engine/loop.hpp"
#include "engine/movement.hpp"
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

TEST(Player, BasisIsSinCos) {
  std::vector<float> angles;
  for (int k = 0; k < 3600; ++k) angles.push_back(static_cast<float>(k) * 0.00174533f);
  angles.push_back(-1.f);
  angles.push_back(100.f);
  for (const float angle : angles) {
    EXPECT_TRUE(bitEqual(riz::game::forwardOf(angle), glm::vec2(std::sin(angle), std::cos(angle)))) << angle;
    EXPECT_TRUE(bitEqual(riz::game::rightOf(angle), glm::vec2(std::cos(angle), -std::sin(angle)))) << angle;
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

static_assert(riz::game::moveDistance(riz::kDefaultStepSeconds) == 0.01f);

TEST(Player, MoveDistanceIsExact) {
  EXPECT_EQ(riz::game::moveDistance(riz::kDefaultStepSeconds), 0.01f);
  EXPECT_EQ(riz::game::moveDistance(0.5), 0.3f);
  EXPECT_EQ(riz::game::moveDistance(0.0), 0.f);
  EXPECT_EQ(riz::game::kMoveSpeed, 0.6);
  EXPECT_EQ(riz::game::kMouseSensitivity, 0.002f);
}

TEST(Player, StepMatchesLegacyFormula) {
  std::vector<float> angles;
  for (int k = 0; k < 360; ++k) angles.push_back(static_cast<float>(k) * 0.01745329f);
  angles.push_back(-1.f);
  angles.push_back(100.f);
  const glm::vec2 positions[] = {glm::vec2(0.f, 0.f), glm::vec2(1.5f, -2.25f)};
  const float axes[] = {-1.f, 0.f, 1.f};
  const float turns[] = {0.f, 3.f, -7.5f};
  for (const float a : angles) {
    for (const glm::vec2 position : positions) {
      for (const float f : axes) {
        for (const float r : axes) {
          for (const float dx : turns) {
            const riz::game::Player p{position, a};
            const riz::game::PlayerCommand c{f, r, dx};
            const riz::game::Player result = riz::game::stepPlayer(p, c, riz::kDefaultStepSeconds);
            const riz::game::Player expected{
                p.position + riz::moveDirection(f, r, riz::game::forwardOf(a), riz::game::rightOf(a)) * 0.01f, a + dx * 0.002f};
            EXPECT_TRUE(bitEqual(result.position, expected.position)) << a << " " << f << " " << r;
            EXPECT_TRUE(bitEqual(result.angle, expected.angle)) << a << " " << dx;
          }
        }
      }
    }
  }
}

TEST(Player, StepScalesWithStepSeconds) {
  const riz::game::Player start{glm::vec2(0.f, 0.f), 0.f};
  const riz::game::Player moved = riz::game::stepPlayer(start, riz::game::PlayerCommand{1.f, 0.f, 0.f}, 0.5);
  EXPECT_EQ(moved.position, glm::vec2(0.f, 0.3f));
  EXPECT_EQ(moved.angle, 0.f);

  const riz::game::Player p{glm::vec2(1.5f, -2.25f), 0.7f};
  const riz::game::Player turned = riz::game::stepPlayer(p, riz::game::PlayerCommand{0.f, 0.f, 10.f}, 0.5);
  EXPECT_EQ(turned.position, p.position);
  EXPECT_EQ(turned.angle, p.angle + 10.f * 0.002f);
}

TEST(Player, CommandFromStepInput) {
  riz::Input input;
  riz::InputLatch latch;
  const auto step = [&]() {
    latch.capture(input);
    return riz::game::playerCommandFrom(latch.take());
  };
  const auto expectCommand = [](const riz::game::PlayerCommand& c, float f, float r, float dx) {
    EXPECT_EQ(c.forwardAxis, f);
    EXPECT_EQ(c.rightAxis, r);
    EXPECT_EQ(c.mouseDeltaX, dx);
  };

  input.beginFrame();
  input.onKey(riz::Key::W, riz::KeyAction::Press);
  input.onKey(riz::Key::D, riz::KeyAction::Press);
  input.onCursorPos(10.0, 5.0);
  input.onCursorPos(13.5, 2.0);
  expectCommand(step(), 1.f, 1.f, 3.5f);

  input.beginFrame();
  input.onKey(riz::Key::S, riz::KeyAction::Press);
  input.onKey(riz::Key::A, riz::KeyAction::Press);
  expectCommand(step(), 0.f, 0.f, 0.f);

  input.beginFrame();
  input.onKey(riz::Key::W, riz::KeyAction::Release);
  expectCommand(step(), -1.f, 0.f, 0.f);

  riz::Input other;
  riz::InputLatch otherLatch;
  other.beginFrame();
  other.onKey(riz::Key::Left, riz::KeyAction::Press);
  other.onKey(riz::Key::Right, riz::KeyAction::Press);
  other.onKey(riz::Key::Up, riz::KeyAction::Press);
  other.onKey(riz::Key::Down, riz::KeyAction::Press);
  other.onKey(riz::Key::Escape, riz::KeyAction::Press);
  otherLatch.capture(other);
  expectCommand(riz::game::playerCommandFrom(otherLatch.take()), 0.f, 0.f, 0.f);
}
