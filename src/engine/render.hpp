#pragma once

#include <string>

#include <glm/vec2.hpp>

#include <vector>
#include "./render/iRender.hpp"

namespace riz {
  class Renderer {
  public:
    Renderer(glm::ivec2 framebufferSize, const std::string& pathVert, const std::string& pathFrag);
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void begin();
    void draw(const std::vector<glm::vec2>& quadVertices);

  private:
    RenderBatch batch;
  };
}
