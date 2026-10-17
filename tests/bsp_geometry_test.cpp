#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "containers/bsp.hpp"
#include "containers/bsp/iBsp.hpp"

using namespace riz::bsp;

TEST(BspGeometry, PointOnSideRightIsFront) {
  const Partition east{{0.f, 0.f}, {1.f, 0.f}};
  EXPECT_EQ(pointOnSide(east, {0.5f, -1.f}), 0);
  EXPECT_EQ(pointOnSide(east, {0.5f, 1.f}), 1);
  const Partition north{{0.f, 0.f}, {0.f, 1.f}};
  EXPECT_EQ(pointOnSide(north, {1.f, 0.5f}), 0);
}

TEST(BspGeometry, PointOnSideOnLineIsBack) {
  const Partition east{{0.f, 0.f}, {1.f, 0.f}};
  EXPECT_EQ(pointOnSide(east, {5.f, 0.f}), 1);
  EXPECT_EQ(pointOnSide(east, {-3.f, 0.f}), 1);
  EXPECT_EQ(pointOnSide(east, {0.f, 0.f}), 1);
  EXPECT_EQ(pointOnSide(east, {1.f, 0.f}), 1);
  EXPECT_EQ(pointOnSide(east, {0.5f, 0.f}), 1);
}

TEST(BspGeometry, SignedDistanceSignAndScale) {
  const glm::dvec2 a(0.0, 0.0);
  const glm::dvec2 b(2.0, 0.0);
  EXPECT_DOUBLE_EQ(signedDistance(a, b, {1.0, -3.0}), 3.0);
  EXPECT_DOUBLE_EQ(signedDistance(a, b, {1.0, 3.0}), -3.0);
}

TEST(BspGeometry, ClassifyFrontBackSplit) {
  const glm::dvec2 a(0.0, 0.0);
  const glm::dvec2 b(10.0, 0.0);
  EXPECT_EQ(classifySegment(a, b, {2.0, -1.0}, {4.0, -3.0}).side, SegSide::Front);
  EXPECT_EQ(classifySegment(a, b, {2.0, 1.0}, {4.0, 3.0}).side, SegSide::Back);
  const SegClass split = classifySegment(a, b, {5.0, -1.0}, {5.0, 1.0});
  EXPECT_EQ(split.side, SegSide::Split);
  EXPECT_GT(split.distA, 0.0);
  EXPECT_LT(split.distB, 0.0);
}

TEST(BspGeometry, ClassifyColinearByDirection) {
  const glm::dvec2 a(0.0, 0.0);
  const glm::dvec2 b(10.0, 0.0);
  EXPECT_EQ(classifySegment(a, b, {1.0, 0.0}, {5.0, 0.0}).side, SegSide::Front);
  EXPECT_EQ(classifySegment(a, b, {5.0, 0.0}, {1.0, 0.0}).side, SegSide::Back);
  const double off = kEpsilon / 2;
  EXPECT_EQ(classifySegment(a, b, {1.0, off}, {5.0, off}).side, SegSide::Front);
  EXPECT_EQ(classifySegment(a, b, {5.0, -off}, {1.0, -off}).side, SegSide::Back);
}

TEST(BspGeometry, ClassifyWithinEpsilonIsNotSplit) {
  const glm::dvec2 a(0.0, 0.0);
  const glm::dvec2 b(10.0, 0.0);
  const double e = kEpsilon / 2;
  EXPECT_EQ(classifySegment(a, b, {1.0, e}, {5.0, -5.0}).side, SegSide::Front);
  EXPECT_EQ(classifySegment(a, b, {1.0, -e}, {5.0, 5.0}).side, SegSide::Back);
}

TEST(BspGeometry, IntersectParam) {
  EXPECT_NEAR(intersectParam({0.0, -1.0}, {0.0, 1.0}, {-1.0, 0.0}, {3.0, 0.0}), 0.25, 1e-12);
}

TEST(BspGeometry, BBoxOfExpandContains) {
  const BBox empty = bboxOf({});
  EXPECT_EQ(empty.min, glm::vec2(0.f, 0.f));
  EXPECT_EQ(empty.max, glm::vec2(0.f, 0.f));

  const BBox box = bboxOf({{1.f, 5.f}, {-2.f, 3.f}, {4.f, -1.f}});
  EXPECT_EQ(box.min, glm::vec2(-2.f, -1.f));
  EXPECT_EQ(box.max, glm::vec2(4.f, 5.f));

  BBox grown = box;
  bboxExpand(grown, {10.f, -7.f});
  EXPECT_EQ(grown.min, glm::vec2(-2.f, -7.f));
  EXPECT_EQ(grown.max, glm::vec2(10.f, 5.f));
  bboxExpand(grown, {0.f, 0.f});
  EXPECT_EQ(grown.max, glm::vec2(10.f, 5.f));

  EXPECT_TRUE(bboxContains(box, {-2.f, -1.f}));
  EXPECT_TRUE(bboxContains(box, {4.f, 5.f}));
  EXPECT_TRUE(bboxContains(box, {0.f, 0.f}));
  EXPECT_FALSE(bboxContains(box, {std::nextafter(4.f, 5.f), 0.f}));
  EXPECT_FALSE(bboxContains(box, {std::nextafter(-2.f, -3.f), 0.f}));
  EXPECT_FALSE(bboxContains(box, {0.f, std::nextafter(5.f, 6.f)}));
  EXPECT_FALSE(bboxContains(box, {0.f, std::nextafter(-1.f, -2.f)}));
}
