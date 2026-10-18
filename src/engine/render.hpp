#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include "./screenQuad.hpp"

namespace riz {
  inline constexpr std::size_t kMaxBatchVertices = 40000;
  inline constexpr std::size_t kMaxBatchQuads = kMaxBatchVertices / 4;
  inline constexpr std::size_t kMaxBatchElements = kMaxBatchQuads * 6;
  static_assert(kMaxBatchVertices % 4 == 0, "batch holds whole quads");

  void loadGlFunctions();

  class RenderBatch;

  class Renderer {
  public:
    Renderer(glm::ivec2 framebufferSize, const std::string& pathVert, const std::string& pathFrag);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void begin();
    void draw(const std::vector<ScreenQuad>& quads);

  private:
    std::unique_ptr<RenderBatch> batch;
  };
}
