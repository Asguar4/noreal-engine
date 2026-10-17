#include <gtest/gtest.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "engine/platform.hpp"
#include "engine/render.hpp"

using riz::RenderBatch;
using riz::RenderLine;
using riz::Renderer;

static_assert(!std::is_copy_constructible_v<RenderBatch> && !std::is_copy_assignable_v<RenderBatch>);
static_assert(std::is_nothrow_move_constructible_v<RenderBatch> && std::is_nothrow_move_assignable_v<RenderBatch>);
static_assert(!std::is_copy_constructible_v<RenderLine> && !std::is_copy_assignable_v<RenderLine>);
static_assert(std::is_nothrow_move_constructible_v<RenderLine> && std::is_nothrow_move_assignable_v<RenderLine>);
static_assert(!std::is_copy_constructible_v<Renderer> && !std::is_move_constructible_v<Renderer>);

namespace {
const std::string kVert = std::string(RIZ_SOURCE_DIR) + "/shaders/default.vert";
const std::string kFrag = std::string(RIZ_SOURCE_DIR) + "/shaders/default.frag";

GLFWwindow* makeWindow() {
  if (glfwInit() != GLFW_TRUE) return nullptr;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  GLFWwindow* w = glfwCreateWindow(64, 64, "t", nullptr, nullptr);
  if (!w) return nullptr;
  glfwMakeContextCurrent(w);
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) return nullptr;
  while (glGetError() != GL_NO_ERROR) {}
  return w;
}

std::vector<glm::vec2> verts(size_t n) {
  std::vector<glm::vec2> v;
  v.resize(n, glm::vec2{0.f, 0.f});
  for (size_t i = 0; i < n; ++i)
    v[i] = glm::vec2{(i % 4 < 2) ? -0.5f : 0.5f, (i % 2) ? -0.5f : 0.5f};
  return v;
}

bool anyVao() {
  for (GLuint i = 1; i < 32; ++i) if (glIsVertexArray(i)) return true;
  return false;
}
bool anyBuf() {
  for (GLuint i = 1; i < 32; ++i) if (glIsBuffer(i)) return true;
  return false;
}
GLuint currentProgram() {
  GLint p = 0;
  glGetIntegerv(GL_CURRENT_PROGRAM, &p);
  return static_cast<GLuint>(p);
}
}

class GL : public ::testing::Test {
protected:
  GLFWwindow* win = nullptr;
  void SetUp() override {
    win = makeWindow();
    if (!win) GTEST_SKIP() << "no GL context available";
  }
  void TearDown() override {
    if (win) glfwDestroyWindow(win);
    glfwTerminate();
  }
};

TEST_F(GL, BatchInitReleaseHandles) {
  RenderBatch b;
  b.init(kVert, kFrag);
  ASSERT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(verts(4));
  GLuint prog = currentProgram();
  EXPECT_TRUE(glIsProgram(prog));
  EXPECT_TRUE(anyBuf());
  b.release();
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  glUseProgram(0);
  EXPECT_FALSE(glIsProgram(prog));
  EXPECT_FALSE(anyVao());
  EXPECT_FALSE(anyBuf());
  b.release();
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL, BatchMove) {
  GLuint prog = 0;
  {
    RenderBatch b;
    b.init(kVert, kFrag);
    b.draw(verts(4));
    prog = currentProgram();
    RenderBatch c(std::move(b));
    {
      RenderBatch dead(std::move(b));
    }
    EXPECT_TRUE(glIsProgram(prog));
    RenderBatch d;
    d = std::move(c);
    EXPECT_TRUE(glIsProgram(prog));
  }
  glUseProgram(0);
  EXPECT_FALSE(glIsProgram(prog));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL, DrawVariousCounts) {
  RenderBatch b;
  b.init(kVert, kFrag);
  b.draw(std::vector<glm::vec2>{});
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(verts(3));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(verts(8));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(verts(4));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(verts(riz::kMaxBatchVertices + 4));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL, DrawPixels) {
  glViewport(0, 0, 64, 64);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  RenderBatch b;
  b.init(kVert, kFrag);
  std::vector<glm::vec2> v{{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};
  b.draw(v);
  unsigned char px[4] = {0, 0, 0, 0};
  glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  EXPECT_TRUE(px[0] || px[1] || px[2]) << "full-screen quad drew nothing";
}

TEST_F(GL, MissingShaderThrowsWithPath) {
  RenderBatch b;
  const std::string bad = "no/such/shader.vert";
  try {
    b.init(bad, kFrag);
    FAIL() << "no throw";
  } catch (const std::runtime_error& e) {
    EXPECT_NE(std::string(e.what()).find(bad), std::string::npos);
  }
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL, CompileErrorHasLogAndCleansUp) {
  auto p = (std::filesystem::temp_directory_path() / "riz_bad.vert").string();
  {
    std::ofstream o(p);
    o << "#version 330 core\nvoid main() { this is not glsl }\n";
  }
  RenderBatch b;
  try {
    b.init(p, kFrag);
    FAIL() << "no throw";
  } catch (const std::runtime_error& e) {
    std::string m = e.what();
    EXPECT_GT(m.size(), p.size() + 30) << m;
  }
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  EXPECT_FALSE(anyVao());
  EXPECT_FALSE(anyBuf());
  b.release();
  std::filesystem::remove(p);
}

TEST_F(GL, RenderLineLifecycle) {
  RenderLine l;
  l.init(kVert, kFrag);
  l.draw(std::vector<glm::vec2>{{0, 0}, {1, 1}});
  l.draw(std::vector<glm::vec2>{});
  l.release();
  l.release();
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST(GLRenderer, ConstructTwiceAndFrame) {
  for (int i = 0; i < 2; ++i) {
    try {
      riz::Window win(riz::WindowConfig{64, 64, "t", false, false, false});
      Renderer r(win.framebufferSize(), kVert, kFrag);
      EXPECT_EQ(glGetError(), GL_NO_ERROR);
      r.begin();
      std::vector<glm::vec2> v{{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};
      win.pollEvents();
      r.draw(v);
      win.swapBuffers();
      EXPECT_EQ(glGetError(), GL_NO_ERROR);
    } catch (const std::runtime_error& e) {
      GTEST_SKIP() << e.what();
    }
  }
}

TEST(GLRenderer, MissingShaderThrowsAndGlfwUsable) {
  {
    try {
      riz::Window win(riz::WindowConfig{64, 64, "t", false, false, false});
      EXPECT_THROW(Renderer(win.framebufferSize(), "no/such.vert", "no/such.frag"), std::runtime_error);
    } catch (const std::runtime_error& e) {
      GTEST_SKIP() << e.what();
    }
  }
  GLFWwindow* w = makeWindow();
  ASSERT_NE(w, nullptr);
  glfwDestroyWindow(w);
  glfwTerminate();
}
