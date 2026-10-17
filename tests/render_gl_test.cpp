#include <gtest/gtest.h>

#include <GL/glew.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "engine/platform.hpp"
#include "engine/render.hpp"
#include "engine/render/iRender.hpp"

using riz::RenderBatch;
using riz::Renderer;

static_assert(!std::is_copy_constructible_v<RenderBatch> && !std::is_copy_assignable_v<RenderBatch>);
static_assert(std::is_nothrow_move_constructible_v<RenderBatch> && std::is_nothrow_move_assignable_v<RenderBatch>);
static_assert(!std::is_copy_constructible_v<Renderer> && !std::is_move_constructible_v<Renderer>);

namespace {
const std::string kVert = std::string(RIZ_SOURCE_DIR) + "/shaders/default.vert";
const std::string kFrag = std::string(RIZ_SOURCE_DIR) + "/shaders/default.frag";

riz::WindowConfig hiddenConfig() {
  return riz::WindowConfig{64, 64, "t", false, false, false};
}

std::vector<riz::WallQuad> quads(size_t n) {
  return std::vector<riz::WallQuad>(n, riz::WallQuad{{-0.5f, 0.5f}, {-0.5f, -0.5f}, {0.5f, 0.5f}, {0.5f, -0.5f}});
}

const riz::WallQuad kFullScreen{{-1, 1}, {-1, -1}, {1, 1}, {1, -1}};

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
  std::unique_ptr<riz::Window> win;
  void SetUp() override {
    try {
      win = std::make_unique<riz::Window>(hiddenConfig());
      riz::loadGlFunctions();
    } catch (const std::runtime_error& e) {
      win.reset();
      GTEST_SKIP() << "no GL context available: " << e.what();
    }
  }
  void TearDown() override {
    win.reset();
  }
};

TEST_F(GL, BatchInitReleaseHandles) {
  RenderBatch b;
  b.init(kVert, kFrag);
  ASSERT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(quads(1));
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
    b.draw(quads(1));
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
  b.draw(std::vector<riz::WallQuad>{});
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(quads(2));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(quads(1));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(quads(riz::kMaxBatchQuads));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  b.draw(quads(riz::kMaxBatchQuads + 1));
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
}

TEST_F(GL, DrawPixels) {
  glViewport(0, 0, 64, 64);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  RenderBatch b;
  b.init(kVert, kFrag);
  b.draw(std::vector<riz::WallQuad>{kFullScreen});
  unsigned char px[4] = {0, 0, 0, 0};
  glReadPixels(32, 32, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
  EXPECT_TRUE(px[0] || px[1] || px[2]) << "full-screen quad drew nothing";
}

TEST_F(GL, DrawPixelsCoversWholeQuad) {
  glViewport(0, 0, 64, 64);
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  RenderBatch b;
  b.init(kVert, kFrag);
  b.draw(std::vector<riz::WallQuad>{kFullScreen});
  const int pts[5][2] = {{32, 32}, {32, 3}, {32, 60}, {3, 32}, {60, 32}};
  for (const auto& p : pts) {
    unsigned char px[4] = {0, 0, 0, 0};
    glReadPixels(p[0], p[1], 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
    EXPECT_TRUE(px[0] || px[1] || px[2]) << p[0] << "," << p[1];
  }
  EXPECT_EQ(glGetError(), GL_NO_ERROR);
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

TEST(GLRenderer, ConstructTwiceAndFrame) {
  for (int i = 0; i < 2; ++i) {
    try {
      riz::Window win(hiddenConfig());
      Renderer r(win.framebufferSize(), kVert, kFrag);
      EXPECT_EQ(glGetError(), GL_NO_ERROR);
      r.begin();
      std::vector<riz::WallQuad> v{kFullScreen};
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
      riz::Window win(hiddenConfig());
      EXPECT_THROW(Renderer(win.framebufferSize(), "no/such.vert", "no/such.frag"), std::runtime_error);
    } catch (const std::runtime_error& e) {
      GTEST_SKIP() << e.what();
    }
  }
  std::unique_ptr<riz::Window> win2;
  ASSERT_NO_THROW(win2 = std::make_unique<riz::Window>(hiddenConfig()));
  ASSERT_NO_THROW(riz::loadGlFunctions());
}

TEST(RenderQuadLayout, IndexPattern) {
  const std::array<GLuint, 6> expected = {0, 1, 3, 0, 3, 2};
  EXPECT_EQ(riz::kQuadIndices, expected);
  EXPECT_EQ(riz::kQuadStartTop, 0u);
  EXPECT_EQ(riz::kQuadStartBottom, 1u);
  EXPECT_EQ(riz::kQuadEndTop, 2u);
  EXPECT_EQ(riz::kQuadEndBottom, 3u);
}

TEST(RenderQuadLayout, VertexOrder) {
  const riz::WallQuad q{{1, 2}, {3, 4}, {5, 6}, {7, 8}};
  const auto v = riz::quadVertices(q);
  EXPECT_EQ(v[0], q.startTop);
  EXPECT_EQ(v[1], q.startBottom);
  EXPECT_EQ(v[2], q.endTop);
  EXPECT_EQ(v[3], q.endBottom);
}
