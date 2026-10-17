#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/vec2.hpp>

namespace riz {
  inline constexpr std::size_t kMaxBatchVertices = 40000;
  inline constexpr std::size_t kMaxBatchElements = kMaxBatchVertices / 4 * 6;
  static_assert(kMaxBatchVertices % 4 == 0, "batch holds whole quads");

  void loadGlFunctions();

  class RenderBatch {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint shader = 0;

    void batchCreate();
  public:
    RenderBatch() = default;
    ~RenderBatch();

    RenderBatch(const RenderBatch&) = delete;
    RenderBatch& operator=(const RenderBatch&) = delete;
    RenderBatch(RenderBatch&& other) noexcept;
    RenderBatch& operator=(RenderBatch&& other) noexcept;

    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const std::vector<glm::vec2>& vertices);
    void release() noexcept;
  };

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
