#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#include <glm/geometric.hpp>
#include <glm/vec2.hpp>

#include "engine/topDownCamera.hpp"
#include "game/automap.hpp"
#include "game/level.hpp"

namespace {
  using riz::game::MapLine;

  constexpr float kPi = 3.14159265358979f;

  void expectNear(glm::vec2 a, glm::vec2 b, float eps) {
    EXPECT_NEAR(a.x, b.x, eps);
    EXPECT_NEAR(a.y, b.y, eps);
  }

  void expectLine(const MapLine& line, glm::vec2 a, glm::vec2 b) {
    EXPECT_EQ(line.a, a);
    EXPECT_EQ(line.b, b);
  }

  bool sameLine(const MapLine& l, const MapLine& r) {
    return l.a == r.a && l.b == r.b;
  }

  riz::TopDownCamera makeCamera() {
    riz::TopDownCamera camera(10.f);
    camera.setViewportSize(glm::ivec2(800, 600));
    return camera;
  }

  glm::vec2 mid(glm::vec2 a, glm::vec2 b) {
    return (a + b) * 0.5f;
  }
}

TEST(Automap, WallLinesFromStart) {
  const riz::game::Level level = riz::game::buildTestLevel();
  std::vector<std::uint32_t> scratch;
  std::vector<MapLine> out;
  riz::game::wallLinesFrontToBack(level.tree, glm::vec2(0.f, 0.f), scratch, out);
  ASSERT_EQ(out.size(), 3u);
  expectLine(out[0], glm::vec2(-4.f, 4.f), glm::vec2(2.f, 4.f));
  expectLine(out[1], glm::vec2(2.f, 4.f), glm::vec2(4.f, 4.f));
  expectLine(out[2], glm::vec2(2.f, 1.f), glm::vec2(2.f, 3.f));
}

TEST(Automap, WallLinesFromEast) {
  const riz::game::Level level = riz::game::buildTestLevel();
  std::vector<std::uint32_t> scratch;
  std::vector<MapLine> out;
  riz::game::wallLinesFrontToBack(level.tree, glm::vec2(5.f, 0.f), scratch, out);
  ASSERT_EQ(out.size(), 3u);
  expectLine(out[0], glm::vec2(2.f, 4.f), glm::vec2(4.f, 4.f));
  expectLine(out[1], glm::vec2(2.f, 1.f), glm::vec2(2.f, 3.f));
  expectLine(out[2], glm::vec2(-4.f, 4.f), glm::vec2(2.f, 4.f));
}

TEST(Automap, WallLinesFollowBspOrder) {
  const riz::game::Level level = riz::game::buildTestLevel();
  std::vector<std::uint32_t> scratch;
  std::vector<MapLine> out;
  const MapLine expected[] = {
    {glm::vec2(-4.f, 4.f), glm::vec2(2.f, 4.f)},
    {glm::vec2(2.f, 4.f), glm::vec2(4.f, 4.f)},
    {glm::vec2(2.f, 1.f), glm::vec2(2.f, 3.f)}};
  for (int xi = -12; xi <= 12; ++xi) {
    if (xi == 4) continue;
    for (int yi = -4; yi <= 16; ++yi) {
      const glm::vec2 p(static_cast<float>(xi) * 0.5f, static_cast<float>(yi) * 0.5f);
      riz::game::wallLinesFrontToBack(level.tree, p, scratch, out);
      ASSERT_EQ(out.size(), 3u);
      const riz::bsp::SubSector& own = level.tree.subSectors[riz::bsp::subSectorAt(level.tree, p)];
      ASSERT_LE(own.segCount, out.size());
      for (std::uint32_t i = 0; i < own.segCount; ++i) {
        const riz::bsp::Seg& seg = level.tree.segs[own.firstSeg + i];
        expectLine(out[i], level.tree.vertices[seg.v1], level.tree.vertices[seg.v2]);
      }
      for (const MapLine& e : expected) {
        EXPECT_EQ(std::count_if(out.begin(), out.end(), [&](const MapLine& l) { return sameLine(l, e); }), 1);
      }
    }
  }
  riz::game::wallLinesFrontToBack(level.tree, glm::vec2(0.f, 0.f), scratch, out);
  const MapLine* data = out.data();
  riz::game::wallLinesFrontToBack(level.tree, glm::vec2(5.f, 0.f), scratch, out);
  EXPECT_EQ(out.data(), data);
}

TEST(Automap, WallLinesEmptyTree) {
  const riz::bsp::Tree tree;
  std::vector<std::uint32_t> scratch;
  std::vector<MapLine> out(3);
  riz::game::wallLinesFrontToBack(tree, glm::vec2(0.f, 0.f), scratch, out);
  EXPECT_TRUE(out.empty());
}

TEST(Automap, PlayerDotIsPixelSquare) {
  riz::TopDownCamera camera = makeCamera();
  camera.setPosition(glm::vec2(1.f, 2.f));
  const riz::WallQuad d = riz::game::dotQuad(camera, glm::vec2(1.f, 2.f), 8.f);
  const glm::vec2 c = camera.worldToScreen(glm::vec2(1.f, 2.f));
  expectNear(c, glm::vec2(0.f, 0.f), 1e-6f);
  expectNear(d.startTop, c + glm::vec2(-0.01f, 8.f / 600.f), 1e-6f);
  expectNear(d.startBottom, c + glm::vec2(-0.01f, -8.f / 600.f), 1e-6f);
  expectNear(d.endTop, c + glm::vec2(0.01f, 8.f / 600.f), 1e-6f);
  expectNear(d.endBottom, c + glm::vec2(0.01f, -8.f / 600.f), 1e-6f);
  EXPECT_NEAR((d.endTop - d.startTop).x * 400.f, 8.f, 1e-4f);
  EXPECT_NEAR((d.startTop - d.startBottom).y * 300.f, 8.f, 1e-4f);
}

TEST(Automap, PlayerDotIgnoresZoomAndAspect) {
  riz::TopDownCamera camera(10.f);
  camera.setViewportSize(glm::ivec2(1024, 512));
  camera.setPosition(glm::vec2(0.f, 0.f));
  for (const float zoom : {2.f, 0.5f}) {
    camera.setZoom(zoom);
    const glm::vec2 world(3.f, -1.f);
    const riz::WallQuad d = riz::game::dotQuad(camera, world, 6.f);
    expectNear(mid(d.startTop, d.endBottom), camera.worldToScreen(world), 1e-6f);
    EXPECT_EQ(d.startTop.x, d.startBottom.x);
    EXPECT_EQ(d.endTop.x, d.endBottom.x);
    EXPECT_EQ(d.startTop.y, d.endTop.y);
    EXPECT_EQ(d.startBottom.y, d.endBottom.y);
    EXPECT_GT(d.endTop.x, d.startTop.x);
    EXPECT_GT(d.startTop.y, d.startBottom.y);
    EXPECT_NEAR((d.endTop.x - d.startTop.x) * 512.f, 6.f, 1e-4f);
    EXPECT_NEAR((d.startTop.y - d.startBottom.y) * 256.f, 6.f, 1e-4f);
  }
}

TEST(Automap, LineQuadThicknessInPixels) {
  riz::TopDownCamera camera = makeCamera();
  const glm::vec2 half(400.f, 300.f);

  const MapLine horizontal{glm::vec2(-2.f, 1.f), glm::vec2(3.f, 1.f)};
  const riz::WallQuad h = riz::game::lineQuad(camera, horizontal, 2.f);
  expectNear(h.startTop - h.startBottom, glm::vec2(0.f, 2.f / 300.f), 1e-6f);
  expectNear(mid(h.startTop, h.startBottom), camera.worldToScreen(horizontal.a), 1e-6f);

  const MapLine vertical{glm::vec2(1.f, -2.f), glm::vec2(1.f, 3.f)};
  const riz::WallQuad v = riz::game::lineQuad(camera, vertical, 2.f);
  EXPECT_NEAR(std::abs((v.startTop - v.startBottom).x), 2.f / 400.f, 1e-6f);
  EXPECT_NEAR((v.startTop - v.startBottom).y, 0.f, 1e-6f);

  camera.setZoom(2.f);
  const MapLine diagonal{glm::vec2(-1.f, -1.f), glm::vec2(1.f, -1.f + 2.f * std::tan(kPi / 6.f))};
  const riz::WallQuad q = riz::game::lineQuad(camera, diagonal, 2.f);
  const glm::vec2 across = (q.startTop - q.startBottom) * half;
  EXPECT_NEAR(glm::length(across), 2.f, 1e-3f);
  const glm::vec2 along = (camera.worldToScreen(diagonal.b) - camera.worldToScreen(diagonal.a)) * half;
  EXPECT_LT(std::abs(glm::dot(across / glm::length(across), along / glm::length(along))), 1e-4f);
}

TEST(Automap, LineQuadDegenerateLine) {
  const riz::TopDownCamera camera = makeCamera();
  const MapLine point{glm::vec2(1.f, 1.f), glm::vec2(1.f, 1.f)};
  const riz::WallQuad q = riz::game::lineQuad(camera, point, 2.f);
  for (const glm::vec2 c : {q.startTop, q.startBottom, q.endTop, q.endBottom}) {
    EXPECT_TRUE(std::isfinite(c.x));
    EXPECT_TRUE(std::isfinite(c.y));
  }
  const glm::vec2 half(400.f, 300.f);
  EXPECT_NEAR(glm::length((q.startTop - q.startBottom) * half), 2.f, 1e-3f);
}

TEST(Automap, LinesToQuadsClearsAndKeepsOrder) {
  const riz::TopDownCamera camera = makeCamera();
  const std::vector<MapLine> lines = {
    {glm::vec2(-4.f, 4.f), glm::vec2(2.f, 4.f)},
    {glm::vec2(2.f, 1.f), glm::vec2(2.f, 3.f)},
    {glm::vec2(0.f, 0.f), glm::vec2(0.f, 0.6f)}};
  std::vector<riz::WallQuad> out(5);
  riz::game::linesToQuads(camera, lines, 3.f, out);
  ASSERT_EQ(out.size(), lines.size());
  for (std::size_t i = 0; i < lines.size(); ++i) {
    const riz::WallQuad expected = riz::game::lineQuad(camera, lines[i], 3.f);
    EXPECT_EQ(std::memcmp(&out[i], &expected, sizeof(riz::WallQuad)), 0);
  }
}

TEST(Automap, CameraCentredOnPlayer) {
  const riz::game::Level level = riz::game::buildTestLevel();
  riz::TopDownCamera camera = makeCamera();
  const glm::vec2 player(1.f, 2.f);
  camera.setPosition(player);
  std::vector<std::uint32_t> scratch;
  std::vector<MapLine> lines;
  std::vector<riz::WallQuad> quads;
  riz::game::wallLinesFrontToBack(level.tree, player, scratch, lines);
  const std::size_t wallCount = lines.size();
  riz::game::linesToQuads(camera, lines, riz::game::kDefaultLineThicknessPixels, quads);
  quads.push_back(riz::game::dotQuad(camera, player, riz::game::kPlayerDotSizePixels));
  ASSERT_EQ(quads.size(), wallCount + 1u);
  const riz::WallQuad& dot = quads.back();
  expectNear(mid(dot.startTop, dot.endBottom), glm::vec2(0.f, 0.f), 1e-6f);

  const glm::vec2 he = camera.visibleHalfExtent();
  const MapLine target{glm::vec2(-4.f, 4.f), glm::vec2(2.f, 4.f)};
  std::size_t found = wallCount;
  for (std::size_t i = 0; i < wallCount; ++i) {
    if (sameLine(lines[i], target)) found = i;
  }
  ASSERT_LT(found, wallCount);
  expectNear(mid(quads[found].startTop, quads[found].startBottom), glm::vec2(-5.f / he.x, 2.f / he.y), 1e-6f);
}
