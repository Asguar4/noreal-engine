#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

#include "containers/bsp/iBsp.hpp"
#include "game/level.hpp"

namespace {
  using riz::game::buildTestLevel;
  using riz::game::testLevelWalls;
}

TEST(TestLevel, WallsAreExact) {
  const std::vector<riz::bsp::Segment> walls = testLevelWalls();
  ASSERT_EQ(walls.size(), 2u);
  EXPECT_EQ(walls[0].a, glm::vec2(-4.f, 4.f));
  EXPECT_EQ(walls[0].b, glm::vec2(4.f, 4.f));
  EXPECT_EQ(walls[1].a, glm::vec2(2.f, 1.f));
  EXPECT_EQ(walls[1].b, glm::vec2(2.f, 3.f));
  EXPECT_FALSE(walls[0].twoSided);
  EXPECT_FALSE(walls[1].twoSided);
}

TEST(TestLevel, BuildSplitsTheCrossingWall) {
  const riz::game::Level level = buildTestLevel();
  const riz::bsp::Tree& tree = level.tree;
  EXPECT_EQ(tree.splitCount, 1u);
  ASSERT_EQ(tree.nodes.size(), 1u);
  EXPECT_EQ(tree.subSectors.size(), 2u);
  EXPECT_EQ(tree.segs.size(), 3u);
  ASSERT_EQ(tree.vertices.size(), 5u);
  EXPECT_EQ(tree.vertices[4], glm::vec2(2.f, 4.f));
  EXPECT_EQ(tree.nodes[0].partition.a, glm::vec2(2.f, 1.f));
  EXPECT_EQ(tree.nodes[0].partition.b, glm::vec2(2.f, 3.f));
  EXPECT_EQ(tree.droppedSegments, 0u);
  EXPECT_EQ(tree.depth, 1u);
  EXPECT_TRUE(riz::bsp::checkInvariants(tree).empty());
}

TEST(TestLevel, PlayerStart) {
  const riz::game::Level level = buildTestLevel();
  EXPECT_EQ(level.playerStart, glm::vec2(0.f, 0.f));
  EXPECT_EQ(level.playerStartAngle, 0.f);
  const std::uint32_t index = riz::bsp::subSectorAt(level.tree, level.playerStart);
  ASSERT_LT(index, level.tree.subSectors.size());
  const riz::bsp::SubSector& subSector = level.tree.subSectors[index];
  ASSERT_EQ(subSector.segCount, 1u);
  const riz::bsp::Seg& seg = level.tree.segs[subSector.firstSeg];
  EXPECT_EQ(level.tree.vertices[seg.v1], glm::vec2(-4.f, 4.f));
  EXPECT_EQ(level.tree.vertices[seg.v2], glm::vec2(2.f, 4.f));
}

TEST(TestLevel, BuildIsDeterministic) {
  const riz::game::Level first = buildTestLevel();
  const riz::game::Level second = buildTestLevel();
  ASSERT_EQ(first.tree.vertices.size(), second.tree.vertices.size());
  for (std::size_t i = 0; i < first.tree.vertices.size(); ++i) {
    EXPECT_EQ(first.tree.vertices[i], second.tree.vertices[i]);
  }
  ASSERT_EQ(first.tree.segs.size(), second.tree.segs.size());
  for (std::size_t i = 0; i < first.tree.segs.size(); ++i) {
    EXPECT_EQ(first.tree.segs[i].v1, second.tree.segs[i].v1);
    EXPECT_EQ(first.tree.segs[i].v2, second.tree.segs[i].v2);
    EXPECT_EQ(first.tree.segs[i].lineDef, second.tree.segs[i].lineDef);
  }
}
