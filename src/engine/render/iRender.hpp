#pragma once

#include <cstddef>
#include <string>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include <vector>

namespace riz {
  inline constexpr std::size_t kMaxBatchVertices = 40000;
  inline constexpr std::size_t kMaxBatchElements = kMaxBatchVertices / 4 * 6;
  static_assert(kMaxBatchVertices % 4 == 0, "batch holds whole quads");

  class RenderLine {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint shader = 0;

    void lineCreate();
  public:
    RenderLine() = default;
    ~RenderLine();

    RenderLine(const RenderLine&) = delete;
    RenderLine& operator=(const RenderLine&) = delete;
    RenderLine(RenderLine&& other) noexcept;
    RenderLine& operator=(RenderLine&& other) noexcept;

    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const std::vector<glm::vec2>& vertices);
    void release() noexcept;
  };

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
}
