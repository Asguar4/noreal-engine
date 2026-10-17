#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "bsp_gen.hpp"
#include "containers/bsp.hpp"
#include "containers/bsp/iBsp.hpp"

using namespace riz::bsp;

namespace {
  Segment seg(float ax, float ay, float bx, float by, bool twoSided = false) {
    Segment s;
    s.a = glm::vec2(ax, ay);
    s.b = glm::vec2(bx, by);
    s.twoSided = twoSided;
    return s;
  }

  bool same(glm::vec2 l, glm::vec2 r) { return l == r; }

  bool same(const BBox& l, const BBox& r) { return l.min == r.min && l.max == r.max; }

  bool same(const Node& l, const Node& r) {
    return l.partition.a == r.partition.a && l.partition.b == r.partition.b && same(l.bbox[0], r.bbox[0]) &&
           same(l.bbox[1], r.bbox[1]) && l.child[0] == r.child[0] && l.child[1] == r.child[1];
  }

  bool same(const Seg& l, const Seg& r) {
    return l.v1 == r.v1 && l.v2 == r.v2 && l.lineDef == r.lineDef && l.side == r.side && l.t1 == r.t1 && l.t2 == r.t2;
  }

  bool same(const LineDef& l, const LineDef& r) {
    return l.v1 == r.v1 && l.v2 == r.v2 && l.flags == r.flags && l.source == r.source;
  }

  bool same(const SubSector& l, const SubSector& r) { return l.firstSeg == r.firstSeg && l.segCount == r.segCount; }

  template <typename T>
  bool sameVec(const std::vector<T>& l, const std::vector<T>& r) {
    if (l.size() != r.size()) return false;
    for (std::size_t i = 0; i < l.size(); ++i) {
      if (!same(l[i], r[i])) return false;
    }
    return true;
  }

  bool sameTree(const Tree& l, const Tree& r) {
    return sameVec(l.vertices, r.vertices) && sameVec(l.lineDefs, r.lineDefs) && sameVec(l.segs, r.segs) &&
           sameVec(l.subSectors, r.subSectors) && sameVec(l.nodes, r.nodes) && l.root == r.root &&
           l.splitCount == r.splitCount && l.droppedSegments == r.droppedSegments && l.depth == r.depth;
  }

  void expectValid(const Tree& tree) {
    const std::vector<std::string> messages = checkInvariants(tree);
    EXPECT_TRUE(messages.empty());
    for (const std::string& m : messages) {
      ADD_FAILURE() << m;
    }
  }

  void expectEmptyTree(const Tree& t) {
    EXPECT_TRUE(t.vertices.empty());
    EXPECT_TRUE(t.lineDefs.empty());
    EXPECT_TRUE(t.segs.empty());
    EXPECT_TRUE(t.nodes.empty());
    ASSERT_EQ(t.subSectors.size(), 1u);
    EXPECT_EQ(t.subSectors[0].firstSeg, 0u);
    EXPECT_EQ(t.subSectors[0].segCount, 0u);
    EXPECT_EQ(t.root, (ChildRef{true, 0}));
    EXPECT_EQ(t.depth, 0u);
    EXPECT_EQ(t.splitCount, 0u);
  }
}

TEST(BspBuild, EmptyInput) {
  const Tree t = build({});
  expectEmptyTree(t);
  EXPECT_EQ(t.droppedSegments, 0u);
  expectValid(t);
}

TEST(BspBuild, DefaultTreeIsEmptyTree) {
  EXPECT_TRUE(sameTree(Tree{}, build({})));
}

TEST(BspBuild, SingleOneSidedSegment) {
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f)});
  EXPECT_EQ(t.vertices.size(), 2u);
  ASSERT_EQ(t.lineDefs.size(), 1u);
  EXPECT_TRUE(same(t.lineDefs[0], LineDef{0, 1, 0, 0}));
  ASSERT_EQ(t.segs.size(), 1u);
  EXPECT_TRUE(same(t.segs[0], Seg{0, 1, 0, 0, 0.f, 1.f}));
  ASSERT_EQ(t.subSectors.size(), 1u);
  EXPECT_TRUE(same(t.subSectors[0], SubSector{0, 1}));
  EXPECT_TRUE(t.nodes.empty());
  EXPECT_EQ(t.root, (ChildRef{true, 0}));
  expectValid(t);
}

TEST(BspBuild, WeldsBitwiseEqualEndpoints) {
  const Tree welded = build({seg(0.f, 0.f, 1.f, 0.f), seg(1.f, 0.f, 1.f, 1.f)});
  EXPECT_EQ(welded.vertices.size(), 3u);
  const Tree apart = build({seg(0.f, 0.f, 1.f, 0.f), seg(std::nextafter(1.f, 2.f), 0.f, 1.f, 1.f)});
  EXPECT_EQ(apart.vertices.size(), 4u);
}

TEST(BspBuild, WeldsNegativeZero) {
  const Tree t = build({seg(-0.f, 0.f, 1.f, 0.f), seg(0.f, 0.f, 0.f, 1.f)});
  EXPECT_EQ(t.vertices.size(), 3u);
  EXPECT_EQ(t.lineDefs[0].v1, t.lineDefs[1].v1);
}

TEST(BspBuild, DropsDegenerateSegments) {
  const float tiny = static_cast<float>(kEpsilon / 2);
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f), seg(3.f, 3.f, 3.f, 3.f), seg(5.f, 5.f, 5.f + tiny, 5.f), seg(0.f, 2.f, 1.f, 2.f)});
  ASSERT_EQ(t.lineDefs.size(), 2u);
  EXPECT_EQ(t.lineDefs[0].source, 0u);
  EXPECT_EQ(t.lineDefs[1].source, 3u);
  EXPECT_EQ(t.droppedSegments, 2u);
  expectValid(t);

  const Tree none = build({seg(3.f, 3.f, 3.f, 3.f), seg(5.f, 5.f, 5.f + tiny, 5.f)});
  expectEmptyTree(none);
  EXPECT_EQ(none.droppedSegments, 2u);
}

TEST(BspBuild, RejectsNonFinite) {
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float inf = std::numeric_limits<float>::infinity();
  for (const float bad : {nan, inf, -inf}) {
    try {
      build({seg(0.f, 0.f, 1.f, 0.f), seg(0.f, 0.f, bad, 1.f)});
      ADD_FAILURE() << "expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
      EXPECT_NE(std::string(e.what()).find("segment 1"), std::string::npos);
    }
  }
}

TEST(BspBuild, RejectsOutOfRange) {
  try {
    build({seg(0.f, 0.f, 4096.5f, 0.f)});
    ADD_FAILURE() << "expected std::invalid_argument";
  } catch (const std::invalid_argument& e) {
    EXPECT_NE(std::string(e.what()).find("segment 0"), std::string::npos);
  }
  EXPECT_NO_THROW(build({seg(-4096.f, -4096.f, 4096.f, 4096.f)}));
}

TEST(BspBuild, SingleTwoSidedSegment) {
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f, true)});
  ASSERT_EQ(t.segs.size(), 2u);
  ASSERT_EQ(t.nodes.size(), 1u);
  EXPECT_EQ(t.nodes[0].partition.a, glm::vec2(0.f, 0.f));
  EXPECT_EQ(t.nodes[0].partition.b, glm::vec2(1.f, 0.f));
  EXPECT_EQ(t.root, (ChildRef{false, 0}));
  EXPECT_EQ(t.nodes[0].child[0], (ChildRef{true, 0}));
  EXPECT_EQ(t.nodes[0].child[1], (ChildRef{true, 1}));
  EXPECT_EQ(t.segs[0].side, 0);
  EXPECT_EQ(t.segs[1].side, 1);
  EXPECT_EQ(t.depth, 1u);
  expectValid(t);
}

TEST(BspBuild, ConvexRoomIsOneSubSector) {
  const Tree t = build({seg(0.f, 0.f, 0.f, 1.f), seg(0.f, 1.f, 1.f, 1.f), seg(1.f, 1.f, 1.f, 0.f), seg(1.f, 0.f, 0.f, 0.f)});
  EXPECT_TRUE(t.nodes.empty());
  ASSERT_EQ(t.subSectors.size(), 1u);
  EXPECT_EQ(t.subSectors[0].segCount, 4u);
  expectValid(t);
}

TEST(BspBuild, OutwardSquareHasFourSubSectors) {
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f), seg(1.f, 0.f, 1.f, 1.f), seg(1.f, 1.f, 0.f, 1.f), seg(0.f, 1.f, 0.f, 0.f)});
  EXPECT_EQ(t.nodes.size(), 3u);
  ASSERT_EQ(t.subSectors.size(), 4u);
  for (const SubSector& ss : t.subSectors) {
    EXPECT_EQ(ss.segCount, 1u);
  }
  EXPECT_EQ(t.splitCount, 0u);
  expectValid(t);
}

TEST(BspBuild, CrossingTwoSidedSegmentsShareSplitVertex) {
  const Tree t = build({seg(-1.f, 0.f, 1.f, 0.f, true), seg(0.f, -1.f, 0.f, 1.f, true)});
  expectValid(t);
  ASSERT_GE(t.vertices.size(), 5u);
  EXPECT_EQ(t.vertices[4], glm::vec2(0.f, 0.f));
  ASSERT_FALSE(t.nodes.empty());
  EXPECT_EQ(t.root, (ChildRef{false, 0}));
  EXPECT_EQ(t.nodes[0].partition.a, glm::vec2(-1.f, 0.f));
  EXPECT_EQ(t.nodes[0].partition.b, glm::vec2(1.f, 0.f));
  EXPECT_EQ(t.splitCount, 4u);
  EXPECT_EQ(t.vertices.size(), 7u);
  EXPECT_EQ(t.segs.size(), 8u);
}

TEST(BspBuild, HeuristicAvoidsSplit) {
  const Tree t = build({seg(-10.f, 0.f, 10.f, 0.f, true), seg(-5.f, 1.f, -5.f, 5.f), seg(5.f, 1.f, 5.f, 5.f)});
  expectValid(t);
  EXPECT_EQ(t.splitCount, 2u);
  ASSERT_FALSE(t.nodes.empty());
  EXPECT_EQ(t.nodes[0].partition.a, glm::vec2(-10.f, 0.f));
  EXPECT_EQ(t.nodes[0].partition.b, glm::vec2(10.f, 0.f));
}

TEST(BspBuild, NearlyTouchingSegmentIsNotSplit) {
  const Tree t = build({seg(0.f, -10.f, 0.f, 10.f, true), seg(-0.0005f, 5.f, 10.f, 5.f)});
  expectValid(t);
  ASSERT_FALSE(t.nodes.empty());
  EXPECT_EQ(t.nodes[0].partition.a, glm::vec2(0.f, -10.f));
  EXPECT_EQ(t.nodes[0].partition.b, glm::vec2(0.f, 10.f));
  std::uint32_t pieces = 0;
  for (const Seg& s : t.segs) {
    if (s.lineDef == 1u) ++pieces;
  }
  EXPECT_EQ(pieces, 1u);
  EXPECT_EQ(t.splitCount, 1u);
  std::vector<ChildRef> stack{t.nodes[0].child[0]};
  bool inFront = false;
  while (!stack.empty()) {
    const ChildRef r = stack.back();
    stack.pop_back();
    if (r.isSubSector) {
      const SubSector& ss = t.subSectors[r.index];
      for (std::uint32_t i = 0; i < ss.segCount; ++i) {
        if (t.segs[ss.firstSeg + i].lineDef == 1u) inFront = true;
      }
    } else {
      stack.push_back(t.nodes[r.index].child[0]);
      stack.push_back(t.nodes[r.index].child[1]);
    }
  }
  EXPECT_TRUE(inFront);
}

TEST(BspBuild, ColinearOverlappingSegments) {
  const Tree sameDir = build({seg(0.f, 0.f, 10.f, 0.f), seg(5.f, 0.f, 15.f, 0.f)});
  expectValid(sameDir);
  const Tree opposite = build({seg(0.f, 0.f, 10.f, 0.f), seg(15.f, 0.f, 5.f, 0.f)});
  expectValid(opposite);
}

TEST(BspBuild, Deterministic) {
  const std::vector<Segment> input = randomSegments(42, 200, false);
  const Tree a = build(input);
  const Tree b = build(input);
  EXPECT_TRUE(sameTree(a, b));
  expectValid(a);
}

TEST(BspBuild, RandomTreesSatisfyInvariants) {
  for (std::uint32_t seed = 1; seed <= 100; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    expectValid(build(randomSegments(seed, 1 + seed % 64, false)));
  }
}

TEST(BspBuild, RandomGridTreesSatisfyInvariants) {
  for (std::uint32_t seed = 1; seed <= 100; ++seed) {
    SCOPED_TRACE("seed " + std::to_string(seed));
    expectValid(build(randomSegments(seed, 1 + seed % 64, true)));
  }
}

TEST(BspBuild, LargeInput) {
  std::vector<Segment> input = randomSegments(7, 1000, false, 60.f);
  for (Segment& s : input) s.twoSided = true;
  Tree t;
  ASSERT_NO_THROW(t = build(input));
  expectValid(t);
}

TEST(BspInvariants, DetectsCorruption) {
  const Tree good = build(randomSegments(11, 30, false));
  ASSERT_TRUE(checkInvariants(good).empty());
  ASSERT_GT(good.nodes.size(), 2u);
  ASSERT_FALSE(good.root.isSubSector);

  {
    Tree t = good;
    t.segs[0].v1 = static_cast<std::uint32_t>(t.vertices.size());
    EXPECT_FALSE(checkInvariants(t).empty());
  }
  {
    Tree t = good;
    t.nodes.back().child[0] = t.root;
    EXPECT_FALSE(checkInvariants(t).empty());
  }
  {
    Tree t = good;
    t.vertices[t.segs[0].v1] += glm::vec2(50.f, 50.f);
    EXPECT_FALSE(checkInvariants(t).empty());
  }
  {
    Tree t = good;
    t.nodes[0].bbox[0].max = t.nodes[0].bbox[0].min;
    EXPECT_FALSE(checkInvariants(t).empty());
  }
  {
    Tree t = good;
    t.segs.pop_back();
    EXPECT_FALSE(checkInvariants(t).empty());
  }
  {
    Tree t = good;
    t.depth += 1;
    EXPECT_FALSE(checkInvariants(t).empty());
  }
}
