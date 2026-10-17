#include <gtest/gtest.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "engine/platform.hpp"
#include "engine/render.hpp"

static_assert(!std::is_copy_constructible_v<riz::Window> && !std::is_copy_assignable_v<riz::Window>);
static_assert(!std::is_move_constructible_v<riz::Window> && !std::is_move_assignable_v<riz::Window>);

namespace {
const std::string kVert = std::string(RIZ_SOURCE_DIR) + "/shaders/default.vert";
const std::string kFrag = std::string(RIZ_SOURCE_DIR) + "/shaders/default.frag";

riz::WindowConfig hiddenConfig() {
  return riz::WindowConfig{64, 64, "t", false, false, false};
}
}

TEST(GLWindow, HiddenLifecycle) {
  try {
    riz::Window win(hiddenConfig());
    EXPECT_NE(glfwGetCurrentContext(), nullptr);
    const glm::ivec2 fb = win.framebufferSize();
    EXPECT_GT(fb.x, 0);
    EXPECT_GT(fb.y, 0);
    EXPECT_FALSE(win.shouldClose());
    win.requestClose();
    EXPECT_TRUE(win.shouldClose());
  } catch (const std::runtime_error& e) {
    GTEST_SKIP() << e.what();
  }
}

TEST(GLWindow, ConstructDestroyConstruct) {
  try {
    { riz::Window first(hiddenConfig()); }
    { riz::Window second(hiddenConfig()); }
    SUCCEED();
  } catch (const std::runtime_error& e) {
    GTEST_SKIP() << e.what();
  }
}

TEST(GLWindow, PollEventsFrameSemantics) {
  try {
    riz::Window win(hiddenConfig());
    win.input().onKey(riz::Key::W, riz::KeyAction::Press);
    win.pollEvents();
    EXPECT_FALSE(win.input().wasPressed(riz::Key::W));
    EXPECT_TRUE(win.input().isDown(riz::Key::W));
    EXPECT_EQ(win.input().mouseDelta(), glm::vec2(0.f, 0.f));
  } catch (const std::runtime_error& e) {
    GTEST_SKIP() << e.what();
  }
}

TEST(GLWindow, WithRenderer) {
  try {
    riz::Window win(hiddenConfig());
    const glm::ivec2 fb = win.framebufferSize();
    riz::Renderer render(fb, kVert, kFrag);

    GLint vp[4] = {-1, -1, -1, -1};
    glGetIntegerv(GL_VIEWPORT, vp);
    EXPECT_EQ(vp[0], 0);
    EXPECT_EQ(vp[1], 0);
    EXPECT_EQ(vp[2], fb.x);
    EXPECT_EQ(vp[3], fb.y);

    GLint major = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    EXPECT_GE(major, 3);
    GLint profile = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    EXPECT_NE(profile & GL_CONTEXT_CORE_PROFILE_BIT, 0);

    std::vector<riz::WallQuad> quad{riz::WallQuad{{-1, 1}, {-1, -1}, {1, 1}, {1, -1}}};
    for (int i = 0; i < 2; ++i) {
      render.begin();
      win.pollEvents();
      render.draw(quad);
      win.swapBuffers();
    }
    EXPECT_EQ(glGetError(), static_cast<GLenum>(GL_NO_ERROR));
  } catch (const std::runtime_error& e) {
    GTEST_SKIP() << e.what();
  }
}

TEST(WindowConfig, DefaultsAndAggregateInit) {
  const riz::WindowConfig c;
  EXPECT_EQ(c.width, 800);
  EXPECT_EQ(c.height, 600);
  EXPECT_EQ(c.title, "OpenGL");
  EXPECT_FALSE(c.resizable);
  EXPECT_TRUE(c.visible);
  EXPECT_TRUE(c.captureCursor);
  EXPECT_FALSE(c.requestVsync);
  EXPECT_FALSE((riz::WindowConfig{64, 64, "t", false, false, false}).requestVsync);
}

TEST(GLWindow, VsyncRequestHiddenSwaps) {
  try {
    const auto start = std::chrono::steady_clock::now();
    auto cfg = hiddenConfig();
    cfg.requestVsync = true;
    riz::Window win(cfg);
    for (int i = 0; i < 3; ++i) {
      win.pollEvents();
      win.swapBuffers();
    }
    EXPECT_FALSE(win.shouldClose());
    const std::chrono::duration<double> elapsed = std::chrono::steady_clock::now() - start;
    EXPECT_LT(elapsed.count(), 5.0);
  } catch (const std::runtime_error& e) {
    GTEST_SKIP() << e.what();
  }
}
