#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../bsp.hpp"
#include "./iBsp.hpp"

namespace riz::bsp {
  namespace {
    struct WorkSeg {
      std::uint32_t v1;
      std::uint32_t v2;
      std::uint32_t lineDef;
      std::uint8_t side;
      double t1;
      double t2;
    };

    struct WorkItem {
      std::vector<WorkSeg> segs;
      std::uint32_t parent;
      std::uint8_t slot;
      std::uint32_t depth;
    };

    enum class Verdict : std::uint8_t { Front, Back, Split };

    struct Decision {
      Verdict verdict = Verdict::Front;
      double tLine = 0.0;
    };

    struct SplitVertex {
      std::uint32_t vertex;
      double tLine;
    };

    struct Candidate {
      std::uint32_t lineDef;
      std::uint8_t side;
    };

    glm::dvec2 toD(glm::vec2 v) noexcept { return glm::dvec2(static_cast<double>(v.x), static_cast<double>(v.y)); }

    Decision decide(const Tree& tree, const WorkSeg& seg, glm::dvec2 l1, glm::dvec2 l2) {
      Decision decision;
      const SegClass cls = classifySegment(l1, l2, toD(tree.vertices[seg.v1]), toD(tree.vertices[seg.v2]));
      if (cls.side == SegSide::Front) {
        decision.verdict = Verdict::Front;
        return decision;
      }
      if (cls.side == SegSide::Back) {
        decision.verdict = Verdict::Back;
        return decision;
      }
      const LineDef& ld = tree.lineDefs[seg.lineDef];
      const glm::dvec2 s1 = toD(tree.vertices[ld.v1]);
      const glm::dvec2 s2 = toD(tree.vertices[ld.v2]);
      const double dL1 = signedDistance(l1, l2, s1);
      const double dL2 = signedDistance(l1, l2, s2);
      const double len = std::sqrt((s2.x - s1.x) * (s2.x - s1.x) + (s2.y - s1.y) * (s2.y - s1.y));
      bool refuse = (dL1 - dL2) == 0.0;
      double tLine = 0.0;
      if (!refuse) {
        tLine = intersectParam(l1, l2, s1, s2);
        const double lo = std::min(seg.t1, seg.t2);
        const double hi = std::max(seg.t1, seg.t2);
        if (!std::isfinite(tLine)) {
          refuse = true;
        } else {
          tLine = std::min(std::max(tLine, lo), hi);
          refuse = std::abs(tLine - seg.t1) * len < kEpsilon || std::abs(seg.t2 - tLine) * len < kEpsilon;
        }
      }
      if (refuse) {
        const double d = std::abs(cls.distA) >= std::abs(cls.distB) ? cls.distA : cls.distB;
        decision.verdict = d > 0.0 ? Verdict::Front : Verdict::Back;
        return decision;
      }
      decision.verdict = Verdict::Split;
      decision.tLine = tLine;
      return decision;
    }

    void candidateLine(const Tree& tree, Candidate c, glm::dvec2& l1, glm::dvec2& l2) {
      const LineDef& ld = tree.lineDefs[c.lineDef];
      const glm::dvec2 p1 = toD(tree.vertices[ld.v1]);
      const glm::dvec2 p2 = toD(tree.vertices[ld.v2]);
      l1 = c.side == 0 ? p1 : p2;
      l2 = c.side == 0 ? p2 : p1;
    }

    bool selectPartition(const Tree& tree, const std::vector<WorkSeg>& segs, Candidate& chosen) {
      if (segs.size() <= 1) {
        return false;
      }
      std::vector<std::uint32_t> keys;
      keys.reserve(segs.size());
      for (const WorkSeg& s : segs) {
        keys.push_back(s.lineDef * 2u + s.side);
      }
      std::sort(keys.begin(), keys.end());
      keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

      bool found = false;
      double bestCost = 0.0;
      for (const std::uint32_t key : keys) {
        const Candidate cand{key / 2u, static_cast<std::uint8_t>(key % 2u)};
        glm::dvec2 l1;
        glm::dvec2 l2;
        candidateLine(tree, cand, l1, l2);
        std::size_t front = 0;
        std::size_t back = 0;
        std::size_t splits = 0;
        bool pruned = false;
        for (const WorkSeg& s : segs) {
          const Decision d = decide(tree, s, l1, l2);
          if (d.verdict == Verdict::Front) {
            ++front;
          } else if (d.verdict == Verdict::Back) {
            ++back;
          } else {
            ++front;
            ++back;
            ++splits;
            if (found && kSplitCost * static_cast<double>(splits) >= bestCost) {
              pruned = true;
              break;
            }
          }
        }
        if (pruned || back == 0) {
          continue;
        }
        const double diff = front >= back ? static_cast<double>(front - back) : static_cast<double>(back - front);
        const double cost = kSplitCost * static_cast<double>(splits) + diff;
        if (!found || cost < bestCost) {
          found = true;
          bestCost = cost;
          chosen = cand;
          if (cost == 0.0) {
            break;
          }
        }
      }
      return found;
    }

    BBox boxOfSegs(const Tree& tree, const std::vector<WorkSeg>& segs) {
      std::vector<glm::vec2> points;
      points.reserve(segs.size() * 2);
      for (const WorkSeg& s : segs) {
        points.push_back(tree.vertices[s.v1]);
        points.push_back(tree.vertices[s.v2]);
      }
      return bboxOf(points);
    }

    void validate(const std::vector<Segment>& segments) {
      for (std::size_t i = 0; i < segments.size(); ++i) {
        const Segment& s = segments[i];
        const float coords[4] = {s.a.x, s.a.y, s.b.x, s.b.y};
        for (const float c : coords) {
          if (!std::isfinite(c)) {
            throw std::invalid_argument("bsp: segment " + std::to_string(i) + " has a non-finite coordinate");
          }
          if (std::abs(c) > kMaxCoord) {
            throw std::invalid_argument("bsp: segment " + std::to_string(i) + " has a coordinate beyond kMaxCoord");
          }
        }
      }
    }

    std::uint32_t bitsOf(float f) noexcept {
      std::uint32_t bits = 0;
      std::memcpy(&bits, &f, sizeof(bits));
      return bits;
    }

    std::vector<WorkSeg> buildInput(const std::vector<Segment>& segments, Tree& tree) {
      std::map<std::pair<std::uint32_t, std::uint32_t>, std::uint32_t> weld;
      const auto weldVertex = [&](glm::vec2 p) {
        if (p.x == 0.f) p.x = 0.f;
        if (p.y == 0.f) p.y = 0.f;
        const auto key = std::make_pair(bitsOf(p.x), bitsOf(p.y));
        const auto it = weld.find(key);
        if (it != weld.end()) {
          return it->second;
        }
        const std::uint32_t index = static_cast<std::uint32_t>(tree.vertices.size());
        tree.vertices.push_back(p);
        weld.emplace(key, index);
        return index;
      };

      std::vector<WorkSeg> work;
      for (std::size_t i = 0; i < segments.size(); ++i) {
        const Segment& s = segments[i];
        const double dx = static_cast<double>(s.b.x) - static_cast<double>(s.a.x);
        const double dy = static_cast<double>(s.b.y) - static_cast<double>(s.a.y);
        if (std::sqrt(dx * dx + dy * dy) < kEpsilon) {
          ++tree.droppedSegments;
          continue;
        }
        const std::uint32_t v1 = weldVertex(s.a);
        const std::uint32_t v2 = weldVertex(s.b);
        LineDef ld;
        ld.v1 = v1;
        ld.v2 = v2;
        ld.flags = s.twoSided ? kLineTwoSided : 0u;
        ld.source = static_cast<std::uint32_t>(i);
        const std::uint32_t index = static_cast<std::uint32_t>(tree.lineDefs.size());
        tree.lineDefs.push_back(ld);
        work.push_back(WorkSeg{v1, v2, index, 0, 0.0, 1.0});
        if (s.twoSided) {
          work.push_back(WorkSeg{v2, v1, index, 1, 1.0, 0.0});
        }
      }
      return work;
    }
  }

  Tree build(const std::vector<Segment>& segments) {
    validate(segments);
    Tree tree;
    std::vector<WorkSeg> initial = buildInput(segments, tree);
    if (initial.empty()) {
      return tree;
    }

    const std::size_t initialCount = initial.size();
    const std::size_t maxDepth = 2 * tree.lineDefs.size() + 2;
    const std::size_t maxSegs = 64 * initialCount + 64;

    std::vector<WorkItem> stack;
    std::size_t workTotal = initial.size();
    stack.push_back(WorkItem{std::move(initial), kNone, 0, 0});

    const auto link = [&tree](std::uint32_t parent, std::uint8_t slot, ChildRef ref) {
      if (parent == kNone) {
        tree.root = ref;
      } else {
        tree.nodes[parent].child[slot] = ref;
      }
    };

    tree.subSectors.clear();

    while (!stack.empty()) {
      WorkItem item = std::move(stack.back());
      stack.pop_back();
      workTotal -= item.segs.size();
      if (item.depth > maxDepth) {
        throw std::runtime_error("bsp: build did not converge (depth guard)");
      }

      Candidate chosen{0, 0};
      if (!selectPartition(tree, item.segs, chosen)) {
        SubSector ss;
        ss.firstSeg = static_cast<std::uint32_t>(tree.segs.size());
        ss.segCount = static_cast<std::uint32_t>(item.segs.size());
        for (const WorkSeg& s : item.segs) {
          Seg seg;
          seg.v1 = s.v1;
          seg.v2 = s.v2;
          seg.lineDef = s.lineDef;
          seg.side = s.side;
          seg.t1 = static_cast<float>(s.t1);
          seg.t2 = static_cast<float>(s.t2);
          tree.segs.push_back(seg);
        }
        const std::uint32_t index = static_cast<std::uint32_t>(tree.subSectors.size());
        tree.subSectors.push_back(ss);
        link(item.parent, item.slot, ChildRef{true, index});
        continue;
      }

      glm::dvec2 l1;
      glm::dvec2 l2;
      candidateLine(tree, chosen, l1, l2);

      const std::uint32_t nodeIndex = static_cast<std::uint32_t>(tree.nodes.size());
      Node node;
      node.partition.a = glm::vec2(static_cast<float>(l1.x), static_cast<float>(l1.y));
      node.partition.b = glm::vec2(static_cast<float>(l2.x), static_cast<float>(l2.y));
      tree.nodes.push_back(node);
      link(item.parent, item.slot, ChildRef{false, nodeIndex});
      tree.depth = std::max(tree.depth, item.depth + 1);

      std::vector<WorkSeg> frontSet;
      std::vector<WorkSeg> backSet;
      std::map<std::uint32_t, SplitVertex> splitCache;

      for (const WorkSeg& s : item.segs) {
        const Decision d = decide(tree, s, l1, l2);
        if (d.verdict == Verdict::Front) {
          frontSet.push_back(s);
          continue;
        }
        if (d.verdict == Verdict::Back) {
          backSet.push_back(s);
          continue;
        }
        SplitVertex sv{};
        const auto it = splitCache.find(s.lineDef);
        if (it != splitCache.end()) {
          sv = it->second;
        } else {
          const LineDef ld = tree.lineDefs[s.lineDef];
          const glm::dvec2 s1 = toD(tree.vertices[ld.v1]);
          const glm::dvec2 s2 = toD(tree.vertices[ld.v2]);
          const glm::dvec2 p = s1 + d.tLine * (s2 - s1);
          sv.vertex = static_cast<std::uint32_t>(tree.vertices.size());
          sv.tLine = d.tLine;
          tree.vertices.push_back(glm::vec2(static_cast<float>(p.x), static_cast<float>(p.y)));
          splitCache.emplace(s.lineDef, sv);
        }
        const SegClass cls = classifySegment(l1, l2, toD(tree.vertices[s.v1]), toD(tree.vertices[s.v2]));
        const WorkSeg first{s.v1, sv.vertex, s.lineDef, s.side, s.t1, sv.tLine};
        const WorkSeg second{sv.vertex, s.v2, s.lineDef, s.side, sv.tLine, s.t2};
        (cls.distA > 0.0 ? frontSet : backSet).push_back(first);
        (cls.distB > 0.0 ? frontSet : backSet).push_back(second);
        ++tree.splitCount;
      }

      if (frontSet.empty() || backSet.empty()) {
        throw std::runtime_error("bsp: build did not converge (empty side)");
      }

      tree.nodes[nodeIndex].bbox[0] = boxOfSegs(tree, frontSet);
      tree.nodes[nodeIndex].bbox[1] = boxOfSegs(tree, backSet);

      workTotal += frontSet.size() + backSet.size();
      if (tree.segs.size() + workTotal > maxSegs) {
        throw std::runtime_error("bsp: build did not converge (seg growth guard)");
      }
      stack.push_back(WorkItem{std::move(backSet), nodeIndex, 1, item.depth + 1});
      stack.push_back(WorkItem{std::move(frontSet), nodeIndex, 0, item.depth + 1});
    }

    return tree;
  }
}
