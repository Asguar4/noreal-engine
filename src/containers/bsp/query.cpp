#include <vector>

#include "./iBsp.hpp"

namespace riz::bsp {
  int pointOnSide(const Partition& partition, glm::vec2 point) noexcept {
    const double ax = partition.a.x;
    const double ay = partition.a.y;
    const double cross = (static_cast<double>(partition.b.x) - ax) * (static_cast<double>(point.y) - ay) -
                         (static_cast<double>(partition.b.y) - ay) * (static_cast<double>(point.x) - ax);
    return cross < 0.0 ? 0 : 1;
  }

  std::uint32_t subSectorAt(const Tree& tree, glm::vec2 point) noexcept {
    ChildRef ref = tree.root;
    while (!ref.isSubSector) {
      const Node& node = tree.nodes[ref.index];
      ref = node.child[static_cast<std::size_t>(pointOnSide(node.partition, point))];
    }
    return ref.index;
  }

  void frontToBack(const Tree& tree, glm::vec2 viewPoint, std::vector<std::uint32_t>& outSubSectors) {
    outSubSectors.clear();
    std::vector<ChildRef> stack;
    stack.reserve(static_cast<std::size_t>(tree.depth) + 1);
    stack.push_back(tree.root);
    while (!stack.empty()) {
      const ChildRef ref = stack.back();
      stack.pop_back();
      if (ref.isSubSector) {
        outSubSectors.push_back(ref.index);
        continue;
      }
      const Node& node = tree.nodes[ref.index];
      const std::size_t s = static_cast<std::size_t>(pointOnSide(node.partition, viewPoint));
      stack.push_back(node.child[s ^ 1u]);
      stack.push_back(node.child[s]);
    }
  }
}
