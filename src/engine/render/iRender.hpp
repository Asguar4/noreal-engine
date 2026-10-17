#pragma once

#include <cstddef>
#include <string>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "../../containers/vector.hpp"

namespace riz {
  inline constexpr std::size_t kMaxBatchVertices = 40000;                        // 10000 quads
  inline constexpr std::size_t kMaxBatchElements = kMaxBatchVertices / 4 * 6;   // 60000
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

    // Throws std::runtime_error on failure.
    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const Vector<glm::vec2>& vertices);
    // Idempotent. The owning GL context must be current when handles are non-zero.
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

    // Throws std::runtime_error on failure.
    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const Vector<glm::vec2>& vertices);
    // Idempotent. The owning GL context must be current when handles are non-zero.
    void release() noexcept;
  };
}
