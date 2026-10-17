#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../bsp.hpp"
#include "./iBsp.hpp"

namespace riz::bsp {
  namespace {
    glm::dvec2 toD(glm::vec2 v) noexcept { return glm::dvec2(static_cast<double>(v.x), static_cast<double>(v.y)); }

    std::string str(std::uint64_t v) { return std::to_string(v); }

    bool checkRanges(const Tree& tree, std::vector<std::string>& out) {
      const std::size_t before = out.size();
      const std::uint64_t vc = tree.vertices.size();
      for (std::size_t i = 0; i < tree.lineDefs.size(); ++i) {
        const LineDef& l = tree.lineDefs[i];
        if (l.v1 >= vc || l.v2 >= vc) {
          out.push_back("I1: linedef " + str(i) + " has a vertex index out of range");
        }
      }
      for (std::size_t i = 0; i < tree.segs.size(); ++i) {
        const Seg& s = tree.segs[i];
        if (s.v1 >= vc || s.v2 >= vc) {
          out.push_back("I1: seg " + str(i) + " has a vertex index out of range");
        }
        if (s.lineDef >= tree.lineDefs.size()) {
          out.push_back("I1: seg " + str(i) + " has a linedef index out of range");
        }
      }
      for (std::size_t i = 0; i < tree.subSectors.size(); ++i) {
        const SubSector& ss = tree.subSectors[i];
        if (static_cast<std::uint64_t>(ss.firstSeg) + ss.segCount > tree.segs.size()) {
          out.push_back("I1: subsector " + str(i) + " range is out of bounds");
        }
      }
      const auto checkChild = [&](ChildRef c, const std::string& where) {
        const std::uint64_t limit = c.isSubSector ? tree.subSectors.size() : tree.nodes.size();
        if (c.index >= limit) {
          out.push_back("I1: " + where + " points out of range");
        }
      };
      checkChild(tree.root, "root");
      for (std::size_t i = 0; i < tree.nodes.size(); ++i) {
        checkChild(tree.nodes[i].child[0], "node " + str(i) + " child 0");
        checkChild(tree.nodes[i].child[1], "node " + str(i) + " child 1");
      }
      return out.size() == before;
    }

    void checkStructure(const Tree& tree, std::vector<std::string>& out) {
      std::vector<std::uint8_t> nodeSeen(tree.nodes.size(), 0);
      std::vector<std::uint8_t> subSeen(tree.subSectors.size(), 0);
      std::vector<std::pair<ChildRef, std::uint32_t>> stack;
      stack.emplace_back(tree.root, 0u);
      std::uint32_t measured = 0;
      while (!stack.empty()) {
        const auto [ref, d] = stack.back();
        stack.pop_back();
        std::vector<std::uint8_t>& seen = ref.isSubSector ? subSeen : nodeSeen;
        if (seen[ref.index] != 0) {
          out.push_back(std::string("I1: ") + (ref.isSubSector ? "subsector " : "node ") + str(ref.index) +
                        " is reachable more than once");
          continue;
        }
        seen[ref.index] = 1;
        if (ref.isSubSector) {
          measured = std::max(measured, d);
        } else {
          for (const ChildRef& c : tree.nodes[ref.index].child) {
            stack.emplace_back(c, d + 1);
          }
        }
      }
      for (std::size_t i = 0; i < nodeSeen.size(); ++i) {
        if (nodeSeen[i] == 0) out.push_back("I1: node " + str(i) + " is not reachable from the root");
      }
      for (std::size_t i = 0; i < subSeen.size(); ++i) {
        if (subSeen[i] == 0) out.push_back("I1: subsector " + str(i) + " is not reachable from the root");
      }
      std::uint64_t cursor = 0;
      for (std::size_t i = 0; i < tree.subSectors.size(); ++i) {
        if (tree.subSectors[i].firstSeg != cursor) {
          out.push_back("I1: subsector " + str(i) + " does not start where the previous one ended");
        }
        cursor = static_cast<std::uint64_t>(tree.subSectors[i].firstSeg) + tree.subSectors[i].segCount;
      }
      if (cursor != tree.segs.size()) {
        out.push_back("I1: subsector ranges do not tile the seg array");
      }
      if (measured != tree.depth) {
        out.push_back("I1: depth is " + str(tree.depth) + " but measured " + str(measured));
      }
    }

    void checkSegs(const Tree& tree, std::vector<std::string>& out) {
      for (std::size_t i = 0; i < tree.segs.size(); ++i) {
        const Seg& s = tree.segs[i];
        const std::string name = "I2: seg " + str(i);
        if (s.v1 == s.v2) {
          out.push_back(name + " has equal vertices");
          continue;
        }
        const glm::dvec2 a = toD(tree.vertices[s.v1]);
        const glm::dvec2 b = toD(tree.vertices[s.v2]);
        if (std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y)) < kEpsilon) {
          out.push_back(name + " is shorter than kEpsilon");
        }
        const LineDef& ld = tree.lineDefs[s.lineDef];
        const glm::dvec2 l1 = toD(tree.vertices[ld.v1]);
        const glm::dvec2 l2 = toD(tree.vertices[ld.v2]);
        if (ld.v1 == ld.v2 || (l1.x == l2.x && l1.y == l2.y)) {
          out.push_back(name + " belongs to a degenerate linedef");
        } else if (std::abs(signedDistance(l1, l2, a)) > kEpsilon || std::abs(signedDistance(l1, l2, b)) > kEpsilon) {
          out.push_back(name + " is not on its linedef line");
        }
        if (!(s.t1 >= 0.f && s.t1 <= 1.f && s.t2 >= 0.f && s.t2 <= 1.f)) {
          out.push_back(name + " has t outside [0, 1]");
        }
        if (s.side == 0) {
          if (!(s.t1 < s.t2)) out.push_back(name + " side 0 needs t1 < t2");
        } else if (s.side == 1) {
          if (!(s.t1 > s.t2)) out.push_back(name + " side 1 needs t1 > t2");
          if ((ld.flags & kLineTwoSided) == 0) out.push_back(name + " is a back seg of a one-sided linedef");
        } else {
          out.push_back(name + " has an invalid side");
        }
      }
    }

    void checkCoverage(const Tree& tree, std::vector<std::string>& out) {
      std::vector<std::vector<std::pair<float, float>>> groups(tree.lineDefs.size() * 2);
      for (const Seg& s : tree.segs) {
        if (s.side > 1) continue;
        groups[static_cast<std::size_t>(s.lineDef) * 2 + s.side].emplace_back(std::min(s.t1, s.t2), std::max(s.t1, s.t2));
      }
      for (std::size_t l = 0; l < tree.lineDefs.size(); ++l) {
        const bool twoSided = (tree.lineDefs[l].flags & kLineTwoSided) != 0;
        for (std::size_t side = 0; side < 2; ++side) {
          auto& g = groups[l * 2 + side];
          const std::string name = "I3: linedef " + str(l) + " side " + str(side);
          if (side == 1 && !twoSided) {
            if (!g.empty()) out.push_back(name + " must have no segs");
            continue;
          }
          if (g.empty()) {
            out.push_back(name + " has no segs");
            continue;
          }
          std::sort(g.begin(), g.end());
          bool ok = g.front().first == 0.f && g.back().second == 1.f;
          for (std::size_t i = 0; ok && i + 1 < g.size(); ++i) {
            ok = g[i].second == g[i + 1].first;
          }
          if (!ok) out.push_back(name + " is not exactly tiled over [0, 1]");
        }
      }
    }

    void collectSubtree(const Tree& tree, ChildRef start, std::vector<std::uint32_t>& subs) {
      std::vector<ChildRef> stack{start};
      while (!stack.empty()) {
        const ChildRef ref = stack.back();
        stack.pop_back();
        if (ref.isSubSector) {
          subs.push_back(ref.index);
        } else {
          stack.push_back(tree.nodes[ref.index].child[1]);
          stack.push_back(tree.nodes[ref.index].child[0]);
        }
      }
    }

    void checkNodes(const Tree& tree, std::vector<std::string>& out) {
      std::vector<std::uint32_t> subs;
      for (std::size_t n = 0; n < tree.nodes.size(); ++n) {
        const Node& node = tree.nodes[n];
        const glm::dvec2 pa = toD(node.partition.a);
        const glm::dvec2 pb = toD(node.partition.b);
        const bool degenerate = pa.x == pb.x && pa.y == pb.y;
        if (degenerate) {
          out.push_back("I4: node " + str(n) + " has a degenerate partition");
        }
        for (std::size_t c = 0; c < 2; ++c) {
          subs.clear();
          collectSubtree(tree, node.child[c], subs);
          bool badSide = false;
          bool badBox = false;
          for (const std::uint32_t ssIndex : subs) {
            const SubSector& ss = tree.subSectors[ssIndex];
            for (std::uint32_t k = 0; k < ss.segCount; ++k) {
              const Seg& s = tree.segs[ss.firstSeg + k];
              for (const std::uint32_t v : {s.v1, s.v2}) {
                const glm::vec2 p = tree.vertices[v];
                if (!degenerate) {
                  const double d = signedDistance(pa, pb, toD(p));
                  if (c == 0 ? d < -kEpsilon : d > kEpsilon) badSide = true;
                }
                if (!bboxContains(node.bbox[c], p)) badBox = true;
              }
            }
          }
          if (badSide) out.push_back("I4: node " + str(n) + " subtree " + str(c) + " crosses its partition");
          if (badBox) out.push_back("I5: node " + str(n) + " bbox " + str(c) + " does not contain its subtree");
        }
      }
    }

    void checkConvex(const Tree& tree, std::vector<std::string>& out) {
      for (std::size_t i = 0; i < tree.subSectors.size(); ++i) {
        const SubSector& ss = tree.subSectors[i];
        bool bad = false;
        for (std::uint32_t a = 0; a < ss.segCount && !bad; ++a) {
          const Seg& s = tree.segs[ss.firstSeg + a];
          const LineDef& ld = tree.lineDefs[s.lineDef];
          const glm::dvec2 p1 = toD(tree.vertices[ld.v1]);
          const glm::dvec2 p2 = toD(tree.vertices[ld.v2]);
          if (p1.x == p2.x && p1.y == p2.y) continue;
          const glm::dvec2 l1 = s.side == 0 ? p1 : p2;
          const glm::dvec2 l2 = s.side == 0 ? p2 : p1;
          for (std::uint32_t b = 0; b < ss.segCount; ++b) {
            if (a == b) continue;
            const Seg& t = tree.segs[ss.firstSeg + b];
            const SegClass cls = classifySegment(l1, l2, toD(tree.vertices[t.v1]), toD(tree.vertices[t.v2]));
            if (cls.side != SegSide::Front) {
              bad = true;
              break;
            }
          }
        }
        if (bad) out.push_back("I6: subsector " + str(i) + " is not convex");
      }
    }
  }

  std::vector<std::string> checkInvariants(const Tree& tree) {
    std::vector<std::string> out;
    if (!checkRanges(tree, out)) {
      return out;
    }
    checkStructure(tree, out);
    if (!out.empty()) {
      return out;
    }
    checkSegs(tree, out);
    checkCoverage(tree, out);
    checkNodes(tree, out);
    checkConvex(tree, out);
    return out;
  }
}
