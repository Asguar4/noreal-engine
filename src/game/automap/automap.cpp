#include "../automap.hpp"

#include <glm/geometric.hpp>

namespace riz::game {
  void wallLinesFrontToBack(const bsp::Tree& tree, glm::vec2 viewPoint,
                            std::vector<std::uint32_t>& subSectorScratch, std::vector<MapLine>& out) {
    out.clear();
    bsp::frontToBack(tree, viewPoint, subSectorScratch);
    for (const std::uint32_t index : subSectorScratch) {
      const bsp::SubSector& subSector = tree.subSectors[index];
      for (std::uint32_t i = 0; i < subSector.segCount; ++i) {
        const bsp::Seg& seg = tree.segs[subSector.firstSeg + i];
        out.push_back(MapLine{tree.vertices[seg.v1], tree.vertices[seg.v2]});
      }
    }
  }

  WallQuad lineQuad(const TopDownCamera& camera, MapLine line, float thicknessPixels) {
    const glm::vec2 sa = camera.worldToScreen(glm::vec2{line.a});
    const glm::vec2 sb = camera.worldToScreen(glm::vec2{line.b});
    const glm::vec2 half = glm::vec2(camera.getViewportSize()) * 0.5f;
    const glm::vec2 d = (sb - sa) * half;
    const float len = glm::length(d);
    const glm::vec2 dir = len > 1e-6f ? d / len : glm::vec2(1.f, 0.f);
    const glm::vec2 n = glm::vec2(-dir.y, dir.x) * (thicknessPixels * 0.5f) / half;
    return WallQuad{sa + n, sa - n, sb + n, sb - n};
  }

  void linesToQuads(const TopDownCamera& camera, const std::vector<MapLine>& lines, float thicknessPixels,
                    std::vector<WallQuad>& out) {
    out.clear();
    out.reserve(lines.size());
    for (const MapLine& line : lines) {
      out.push_back(lineQuad(camera, line, thicknessPixels));
    }
  }

  WallQuad dotQuad(const TopDownCamera& camera, glm::vec2 position, float sizePixels) {
    const glm::vec2 c = camera.worldToScreen(position);
    const glm::vec2 h = glm::vec2(sizePixels) / glm::vec2(camera.getViewportSize());
    return WallQuad{c + glm::vec2(-h.x, h.y), c + glm::vec2(-h.x, -h.y),
                    c + glm::vec2(h.x, h.y), c + glm::vec2(h.x, -h.y)};
  }
}
