#include <gtest/gtest.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <random>
#include <set>
#include <type_traits>

#include "engine/input.hpp"
#include "engine/platform/iPlatform.hpp"

using riz::Input;
using riz::Key;
using riz::KeyAction;

namespace {
bool sameBits(float a, float b) {
  std::uint32_t x = 0;
  std::uint32_t y = 0;
  std::memcpy(&x, &a, sizeof x);
  std::memcpy(&y, &b, sizeof y);
  return x == y;
}
}  // namespace

// ---- every public member of Input is noexcept ----
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
  // and no real key was touched
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
  in.onCursorPos(105.5, 50.0);   // relative to the kept last position (104.5, 48)
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

// ---- GLFW key mapping ----
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

// ---- equivalence with the old main.cpp callbacks (commit 922835b) ----
namespace {
// Reference model: the old callbacks' logic, verbatim, on ints/doubles.
struct LegacyInput {
  bool w = false, a = false, s = false, d = false, o = false, p = false;
  double lastX = 0.0;
  double lastY = 0.0;
  float mousedx = 0.f;
  float mousedy = 0.f;
  bool firstMouse = true;
  bool close = false;

  void onKey(int key, int action) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      close = true;
    }
    switch (key) {
      case GLFW_KEY_W: w = action != GLFW_RELEASE; break;
      case GLFW_KEY_A: a = action != GLFW_RELEASE; break;
      case GLFW_KEY_S: s = action != GLFW_RELEASE; break;
      case GLFW_KEY_D: d = action != GLFW_RELEASE; break;
      case GLFW_KEY_O: o = action != GLFW_RELEASE; break;
      case GLFW_KEY_P: p = action != GLFW_RELEASE; break;
      default: break;
    }
  }

  void onCursor(double xpos, double ypos) {
    if (firstMouse) {
      lastX = xpos;
      lastY = ypos;
      firstMouse = false;
    }
    mousedx += static_cast<float>(xpos - lastX);
    mousedy += static_cast<float>(lastY - ypos);
    lastX = xpos;
    lastY = ypos;
  }
};
}  // namespace

TEST(InputLegacy, EquivalentToOldCallbacks) {
  const int keys[] = {GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_O,
                      GLFW_KEY_P, GLFW_KEY_ESCAPE, GLFW_KEY_F1, GLFW_KEY_UNKNOWN};
  const int actions[] = {GLFW_PRESS, GLFW_REPEAT, GLFW_RELEASE};

  std::mt19937 rng(12345u);
  std::uniform_int_distribution<int> eventCount(0, 6);
  std::uniform_int_distribution<int> kind(0, 1);
  std::uniform_int_distribution<int> keyPick(0, 8);
  std::uniform_int_distribution<int> actionPick(0, 2);
  std::uniform_real_distribution<double> coord(-1e6, 1e6);

  LegacyInput old;
  Input in;

  for (int frame = 0; frame < 2000; ++frame) {
    // old frame protocol: read, then zero, then the poll delivers events
    old.mousedx = 0.f;
    old.close = false;
    in.beginFrame();

    const int n = eventCount(rng);
    for (int e = 0; e < n; ++e) {
      if (kind(rng) == 0) {
        const int key = keys[keyPick(rng)];
        const int action = actions[actionPick(rng)];
        old.onKey(key, action);
        in.onKey(riz::keyFromGlfw(key), riz::keyActionFromGlfw(action));
      } else {
        const double x = coord(rng);
        const double y = coord(rng);
        old.onCursor(x, y);
        in.onCursorPos(x, y);
      }
    }

    ASSERT_EQ(in.isDown(Key::W), old.w) << frame;
    ASSERT_EQ(in.isDown(Key::A), old.a) << frame;
    ASSERT_EQ(in.isDown(Key::S), old.s) << frame;
    ASSERT_EQ(in.isDown(Key::D), old.d) << frame;
    ASSERT_TRUE(sameBits(in.mouseDelta().x, old.mousedx)) << frame;
    ASSERT_EQ(in.wasPressed(Key::Escape), old.close) << frame;
  }
}
