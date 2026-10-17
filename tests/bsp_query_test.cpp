#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
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

  struct Constraint {
    std::uint32_t node;
    int branch;
  };

  struct PathItem {
    ChildRef ref;
    std::vector<Constraint> path;
  };

  std::vector<std::vector<Constraint>> leafConstraints(const Tree& tree) {
    std::vector<std::vector<Constraint>> result(tree.subSectors.size());
    std::vector<PathItem> stack;
    stack.push_back(PathItem{tree.root, {}});
    while (!stack.empty()) {
      PathItem item = std::move(stack.back());
      stack.pop_back();
      if (item.ref.isSubSector) {
        result[item.ref.index] = item.path;
        continue;
      }
      for (int branch = 0; branch < 2; ++branch) {
        PathItem next{tree.nodes[item.ref.index].child[static_cast<std::size_t>(branch)], item.path};
        next.path.push_back(Constraint{item.ref.index, branch});
        stack.push_back(std::move(next));
      }
    }
    return result;
  }

  std::vector<Tree> sampleTrees(std::uint32_t count) {
    std::vector<Tree> trees;
    for (std::uint32_t i = 0; i < count; ++i) {
      trees.push_back(build(randomSegments(100 + i, 5 + i % 40, i % 2 == 1)));
    }
    return trees;
  }

  std::size_t extremeOrder(const Tree& t, ChildRef start, const std::vector<std::size_t>& order, bool wantMax) {
    std::size_t result = wantMax ? 0 : order.size();
    std::vector<ChildRef> stack{start};
    while (!stack.empty()) {
      const ChildRef r = stack.back();
      stack.pop_back();
      if (r.isSubSector) {
        result = wantMax ? std::max(result, order[r.index]) : std::min(result, order[r.index]);
      } else {
        stack.push_back(t.nodes[r.index].child[0]);
        stack.push_back(t.nodes[r.index].child[1]);
      }
    }
    return result;
  }
}

TEST(BspLocate, EmptyTreeReturnsZero) {
  const Tree t = build({});
  EXPECT_EQ(subSectorAt(t, {0.f, 0.f}), 0u);
  EXPECT_EQ(subSectorAt(t, {123.f, -456.f}), 0u);
}

TEST(BspLocate, TwoSidedLineSides) {
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f, true)});
  EXPECT_EQ(subSectorAt(t, {0.5f, -1.f}), 0u);
  EXPECT_EQ(subSectorAt(t, {0.5f, 1.f}), 1u);
}

TEST(BspLocate, OnLineGoesBack) {
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f, true)});
  EXPECT_EQ(subSectorAt(t, {0.5f, 0.f}), 1u);
  EXPECT_EQ(subSectorAt(t, {5.f, 0.f}), 1u);
}

TEST(BspLocate, MatchesLeafConstraints) {
  std::mt19937 rng(5);
  std::uniform_real_distribution<float> coord(-1100.f, 1100.f);
  for (std::uint32_t i = 0; i < 50; ++i) {
    SCOPED_TRACE("tree " + std::to_string(i));
    const Tree t = build(randomSegments(500 + i, 3 + i % 50, i % 2 == 1));
    const auto constraints = leafConstraints(t);
    for (int p = 0; p < 200; ++p) {
      const glm::vec2 point(coord(rng), coord(rng));
      std::vector<std::uint32_t> matching;
      for (std::uint32_t s = 0; s < constraints.size(); ++s) {
        bool all = true;
        for (const Constraint& c : constraints[s]) {
          if (pointOnSide(t.nodes[c.node].partition, point) != c.branch) {
            all = false;
            break;
          }
        }
        if (all) matching.push_back(s);
      }
      ASSERT_EQ(matching.size(), 1u);
      EXPECT_EQ(subSectorAt(t, point), matching[0]);
    }
  }
}

TEST(BspTraverse, EmptyTreeYieldsRoot) {
  const Tree t = build({});
  std::vector<std::uint32_t> out;
  frontToBack(t, {1.f, 1.f}, out);
  EXPECT_EQ(out, (std::vector<std::uint32_t>{0}));
}

TEST(BspTraverse, TwoSidedLineOrder) {
  const Tree t = build({seg(0.f, 0.f, 1.f, 0.f, true)});
  std::vector<std::uint32_t> out;
  frontToBack(t, {0.5f, -1.f}, out);
  EXPECT_EQ(out, (std::vector<std::uint32_t>{0, 1}));
  frontToBack(t, {0.5f, 1.f}, out);
  EXPECT_EQ(out, (std::vector<std::uint32_t>{1, 0}));
}

TEST(BspTraverse, VisitsEverySubSectorOnceNearFirst) {
  std::mt19937 rng(9);
  std::uniform_real_distribution<float> coord(-1100.f, 1100.f);
  const std::vector<Tree> trees = sampleTrees(30);
  for (const Tree& t : trees) {
    std::vector<std::uint32_t> out;
    for (int v = 0; v < 10; ++v) {
      const glm::vec2 view(coord(rng), coord(rng));
      frontToBack(t, view, out);
      std::vector<std::uint32_t> sorted = out;
      std::sort(sorted.begin(), sorted.end());
      ASSERT_EQ(sorted.size(), t.subSectors.size());
      for (std::uint32_t i = 0; i < sorted.size(); ++i) ASSERT_EQ(sorted[i], i);
      EXPECT_EQ(out[0], subSectorAt(t, view));

      std::vector<std::size_t> order(t.subSectors.size());
      for (std::size_t i = 0; i < out.size(); ++i) order[out[i]] = i;
      for (const Node& node : t.nodes) {
        const std::size_t near = static_cast<std::size_t>(pointOnSide(node.partition, view));
        const std::size_t nearMax = extremeOrder(t, node.child[near], order, true);
        const std::size_t farMin = extremeOrder(t, node.child[near ^ 1u], order, false);
        ASSERT_LT(nearMax, farMin);
      }
    }
  }
}

TEST(BspTraverse, RayOcclusionOrder) {
  std::mt19937 rng(21);
  std::uniform_real_distribution<float> coord(-1100.f, 1100.f);
  std::uniform_real_distribution<double> angle(0.0, 6.283185307179586);
  const double tol = 1e-3;
  for (std::uint32_t i = 0; i < 30; ++i) {
    SCOPED_TRACE("tree " + std::to_string(i));
    const Tree t = build(randomSegments(900 + i, 5 + i % 40, i % 2 == 1));
    std::vector<std::uint32_t> segSub(t.segs.size(), 0);
    for (std::uint32_t s = 0; s < t.subSectors.size(); ++s) {
      for (std::uint32_t k = 0; k < t.subSectors[s].segCount; ++k) segSub[t.subSectors[s].firstSeg + k] = s;
    }
    std::vector<std::uint32_t> out;
    for (int v = 0; v < 20; ++v) {
      const glm::vec2 view(coord(rng), coord(rng));
      frontToBack(t, view, out);
      std::vector<std::size_t> order(t.subSectors.size());
      for (std::size_t k = 0; k < out.size(); ++k) order[out[k]] = k;
      for (int r = 0; r < 64; ++r) {
        const double a = angle(rng);
        const double dx = std::cos(a);
        const double dy = std::sin(a);
        std::vector<std::pair<double, std::size_t>> hits;
        for (std::size_t s = 0; s < t.segs.size(); ++s) {
          const glm::dvec2 p1(t.vertices[t.segs[s].v1]);
          const glm::dvec2 p2(t.vertices[t.segs[s].v2]);
          const double ex = p2.x - p1.x;
          const double ey = p2.y - p1.y;
          const double denom = dx * ey - dy * ex;
          if (std::abs(denom) < 1e-12) continue;
          const double wx = p1.x - view.x;
          const double wy = p1.y - view.y;
          const double dist = (wx * ey - wy * ex) / denom;
          const double u = (wx * dy - wy * dx) / denom;
          if (dist <= 0.0 || u < 0.0 || u > 1.0) continue;
          const double len = std::sqrt(ex * ex + ey * ey);
          if (u * len < tol || (1.0 - u) * len < tol) continue;
          hits.emplace_back(dist, order[segSub[s]]);
        }
        for (std::size_t x = 0; x < hits.size(); ++x) {
          for (std::size_t y = 0; y < hits.size(); ++y) {
            if (hits[x].first + tol < hits[y].first) {
              ASSERT_LE(hits[x].second, hits[y].second);
            }
          }
        }
      }
    }
  }
}

TEST(BspTraverse, ReusesOutCapacity) {
  const Tree t = build(randomSegments(3, 40, false));
  std::vector<std::uint32_t> out;
  frontToBack(t, {0.f, 0.f}, out);
  const std::size_t capacity = out.capacity();
  const std::uint32_t* data = out.data();
  frontToBack(t, {100.f, -50.f}, out);
  EXPECT_EQ(out.capacity(), capacity);
  EXPECT_EQ(out.data(), data);
}
