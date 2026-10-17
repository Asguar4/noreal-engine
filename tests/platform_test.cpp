#include <gtest/gtest.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstddef>
#include <set>

#include "engine/input.hpp"
#include "engine/platform/iPlatform.hpp"

using riz::Key;
using riz::KeyAction;

TEST(KeyMap, LettersDigitsAndSpecials) {
  for (int i = 0; i < 26; ++i) {
    EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_A + i), static_cast<Key>(static_cast<int>(Key::A) + i)) << i;
  }
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_0 + i), static_cast<Key>(static_cast<int>(Key::Num0) + i)) << i;
  }
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_W), Key::W);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_Z), Key::Z);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_SPACE), Key::Space);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_ESCAPE), Key::Escape);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_ENTER), Key::Enter);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_TAB), Key::Tab);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_BACKSPACE), Key::Backspace);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_LEFT), Key::Left);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_RIGHT), Key::Right);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_UP), Key::Up);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_DOWN), Key::Down);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_LEFT_SHIFT), Key::LeftShift);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_RIGHT_SHIFT), Key::RightShift);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_LEFT_CONTROL), Key::LeftControl);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_RIGHT_CONTROL), Key::RightControl);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_LEFT_ALT), Key::LeftAlt);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_RIGHT_ALT), Key::RightAlt);
}

TEST(KeyMap, InjectiveAndSurjective) {
  std::set<int> seen;
  for (int code = -10; code <= GLFW_KEY_LAST + 10; ++code) {
    const Key k = riz::keyFromGlfw(code);
    if (k == Key::Unknown) continue;
    EXPECT_LT(static_cast<int>(k), static_cast<int>(Key::Count)) << code;
    EXPECT_TRUE(seen.insert(static_cast<int>(k)).second) << "key produced twice, code " << code;
  }
  EXPECT_EQ(seen.size(), static_cast<std::size_t>(Key::Count) - 1);
  for (int k = 1; k < static_cast<int>(Key::Count); ++k) {
    EXPECT_EQ(seen.count(k), 1u) << k;
  }
}

TEST(KeyMap, UnmappedIsUnknown) {
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_UNKNOWN), Key::Unknown);
  EXPECT_EQ(riz::keyFromGlfw(GLFW_KEY_F1), Key::Unknown);
  EXPECT_EQ(riz::keyFromGlfw(-5), Key::Unknown);
  EXPECT_EQ(riz::keyFromGlfw(100000), Key::Unknown);
}

TEST(KeyMap, Actions) {
  EXPECT_EQ(riz::keyActionFromGlfw(GLFW_PRESS), KeyAction::Press);
  EXPECT_EQ(riz::keyActionFromGlfw(GLFW_REPEAT), KeyAction::Repeat);
  EXPECT_EQ(riz::keyActionFromGlfw(GLFW_RELEASE), KeyAction::Release);
  EXPECT_EQ(riz::keyActionFromGlfw(42), KeyAction::Release);
}
