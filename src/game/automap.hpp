#pragma once

#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>

#include "../containers/bsp.hpp"
#include "../engine/topDownCamera.hpp"
#include "../engine/screenQuad.hpp"

namespace riz::game {
  inline constexpr float kMapViewHeight = 10.f;
  inline constexpr float kWallLineThicknessPixels = 2.f;
  inline constexpr float kPlayerDotSizePixels = 8.f;

  struct MapLine {
    glm::vec2 a{0.f, 0.f};
    glm::vec2 b{0.f, 0.f};
  };

  void wallLinesFrontToBack(const bsp::Tree& tree, glm::vec2 viewPoint,
                            std::vector<std::uint32_t>& subSectorScratch, std::vector<MapLine>& out);
  ScreenQuad lineQuad(const TopDownCamera& camera, MapLine line, float thicknessPixels);
  void linesToQuads(const TopDownCamera& camera, const std::vector<MapLine>& lines, float thicknessPixels,
                    std::vector<ScreenQuad>& out);
  ScreenQuad dotQuad(const TopDownCamera& camera, glm::vec2 position, float sizePixels);
}
