#include <gtest/gtest.h>

#include <cstddef>
#include <type_traits>

#include "engine/input.hpp"

using riz::Input;
using riz::Key;
using riz::KeyAction;

static_assert(noexcept(std::declval<const Input&>().isDown(Key::A)));
static_assert(noexcept(std::declval<const Input&>().wasPressed(Key::A)));
static_assert(noexcept(std::declval<const Input&>().wasReleased(Key::A)));
static_assert(noexcept(std::declval<const Input&>().mouseDelta()));
static_assert(noexcept(std::declval<Input&>().beginFrame()));
static_assert(noexcept(std::declval<Input&>().onKey(Key::A, KeyAction::Press)));
static_assert(noexcept(std::declval<Input&>().onCursorPos(0.0, 0.0)));

TEST(Input, DefaultState) {
  Input in;
  for (std::size_t k = 0; k <= static_cast<std::size_t>(Key::Count); ++k) {
    const Key key = static_cast<Key>(k);
    EXPECT_FALSE(in.isDown(key)) << k;
    EXPECT_FALSE(in.wasPressed(key)) << k;
    EXPECT_FALSE(in.wasReleased(key)) << k;
  }
  EXPECT_EQ(in.mouseDelta(), glm::vec2(0.f, 0.f));
}

TEST(Input, PressSetsDownAndPressedBeginFrameClearsPressed) {
  Input in;
  in.onKey(Key::W, KeyAction::Press);
  EXPECT_TRUE(in.isDown(Key::W));
  EXPECT_TRUE(in.wasPressed(Key::W));
  EXPECT_FALSE(in.wasReleased(Key::W));
  in.beginFrame();
  EXPECT_TRUE(in.isDown(Key::W));
  EXPECT_FALSE(in.wasPressed(Key::W));
}

TEST(Input, RepeatSetsDownButNotPressed) {
  Input in;
  in.onKey(Key::A, KeyAction::Repeat);
  EXPECT_TRUE(in.isDown(Key::A));
  EXPECT_FALSE(in.wasPressed(Key::A));
}

TEST(Input, PressThenReleaseInOnePollIsLatched) {
  Input in;
  in.beginFrame();
  in.onKey(Key::Escape, KeyAction::Press);
  in.onKey(Key::Escape, KeyAction::Release);
  EXPECT_TRUE(in.wasPressed(Key::Escape));
  EXPECT_TRUE(in.wasReleased(Key::Escape));
  EXPECT_FALSE(in.isDown(Key::Escape));
  in.beginFrame();
  EXPECT_FALSE(in.wasPressed(Key::Escape));
  EXPECT_FALSE(in.wasReleased(Key::Escape));
}

TEST(Input, ReleaseClearsDown) {
  Input in;
  in.onKey(Key::S, KeyAction::Press);
  in.beginFrame();
  in.onKey(Key::S, KeyAction::Release);
  EXPECT_FALSE(in.isDown(Key::S));
  EXPECT_TRUE(in.wasReleased(Key::S));
}

TEST(Input, InvalidKeysIgnored) {
  Input in;
  const Key bad[] = {Key::Unknown, Key::Count, static_cast<Key>(200), static_cast<Key>(255)};
  for (Key k : bad) {
    in.onKey(k, KeyAction::Press);
    in.onKey(k, KeyAction::Repeat);
    in.onKey(k, KeyAction::Release);
    in.onKey(k, KeyAction::Press);
    EXPECT_FALSE(in.isDown(k));
    EXPECT_FALSE(in.wasPressed(k));
    EXPECT_FALSE(in.wasReleased(k));
  }
  for (std::size_t k = 1; k < static_cast<std::size_t>(Key::Count); ++k) {
    EXPECT_FALSE(in.isDown(static_cast<Key>(k))) << k;
  }
}

TEST(Input, CursorFirstEventIsZeroThenSums) {
  Input in;
  in.onCursorPos(100.0, 50.0);
  EXPECT_EQ(in.mouseDelta(), glm::vec2(0.f, 0.f));
  in.onCursorPos(103.0, 48.0);
  in.onCursorPos(104.5, 48.0);
  EXPECT_EQ(in.mouseDelta(), glm::vec2(4.5f, -2.f));
  in.beginFrame();
  EXPECT_EQ(in.mouseDelta(), glm::vec2(0.f, 0.f));
  in.onCursorPos(105.5, 50.0);
  EXPECT_EQ(in.mouseDelta(), glm::vec2(1.f, 2.f));
}

TEST(Input, CursorBitExactFloatSum) {
  const double xs[] = {0.1, 0.30000000000000004, 1e9 + 0.25, -3.75};
  const double ys[] = {2.5, 1e-3, -7.125, 1e9 - 0.5};
  Input in;
  float ex = 0.f;
  float ey = 0.f;
  for (int i = 0; i < 4; ++i) {
    in.onCursorPos(xs[i], ys[i]);
    if (i > 0) {
      ex += static_cast<float>(xs[i] - xs[i - 1]);
      ey += static_cast<float>(ys[i] - ys[i - 1]);
    }
  }
  EXPECT_TRUE(in.mouseDelta().x == ex);
  EXPECT_TRUE(in.mouseDelta().y == ey);
}
