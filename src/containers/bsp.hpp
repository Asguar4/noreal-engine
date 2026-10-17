#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

namespace riz::bsp {
  inline constexpr std::uint32_t kNone = 0xFFFFFFFFu;
  inline constexpr std::uint32_t kLineTwoSided = 4u;
  inline constexpr float kMaxCoord = 4096.f;

  struct Segment {
    glm::vec2 a{0.f, 0.f};
    glm::vec2 b{0.f, 0.f};
    bool twoSided = false;
  };

  using Vertex = glm::vec2;

  struct LineDef {
    std::uint32_t v1 = kNone;
    std::uint32_t v2 = kNone;
    std::uint32_t flags = 0;
    std::uint32_t source = kNone;
  };

  struct Seg {
    std::uint32_t v1 = kNone;
    std::uint32_t v2 = kNone;
    std::uint32_t lineDef = kNone;
    std::uint8_t side = 0;
    float t1 = 0.f;
    float t2 = 1.f;
  };

  struct SubSector {
    std::uint32_t firstSeg = 0;
    std::uint32_t segCount = 0;
  };

  struct BBox {
    glm::vec2 min{0.f, 0.f};
    glm::vec2 max{0.f, 0.f};
  };

  struct Partition {
    glm::vec2 a{0.f, 0.f};
    glm::vec2 b{0.f, 0.f};
  };

  struct ChildRef {
    bool isSubSector = true;
    std::uint32_t index = 0;
  };

  constexpr bool operator==(ChildRef l, ChildRef r) noexcept { return l.isSubSector == r.isSubSector && l.index == r.index; }
  constexpr bool operator!=(ChildRef l, ChildRef r) noexcept { return !(l == r); }

  struct Node {
    Partition partition;
    std::array<BBox, 2> bbox{};
    std::array<ChildRef, 2> child{};
  };

  struct Tree {
    std::vector<Vertex> vertices;
    std::vector<LineDef> lineDefs;
    std::vector<Seg> segs;
    std::vector<SubSector> subSectors{SubSector{}};
    std::vector<Node> nodes;
    ChildRef root{};
    std::uint32_t splitCount = 0;
    std::uint32_t droppedSegments = 0;
    std::uint32_t depth = 0;
  };

  Tree build(const std::vector<Segment>& segments);

  int pointOnSide(const Partition& partition, glm::vec2 point) noexcept;
  std::uint32_t subSectorAt(const Tree& tree, glm::vec2 point) noexcept;
  void frontToBack(const Tree& tree, glm::vec2 viewPoint, std::vector<std::uint32_t>& outSubSectors);
}
