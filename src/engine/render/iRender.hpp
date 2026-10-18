#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "../render.hpp"
#include "../screenQuad.hpp"

namespace riz {
  template <typename Traits>
  class GlHandle {
    GLuint id = 0;
  public:
    GlHandle() noexcept = default;
    explicit GlHandle(GLuint handle) noexcept : id(handle) {}
    ~GlHandle() { reset(); }

    GlHandle(const GlHandle&) = delete;
    GlHandle& operator=(const GlHandle&) = delete;
    GlHandle(GlHandle&& other) noexcept : id(std::exchange(other.id, 0)) {}
    GlHandle& operator=(GlHandle&& other) noexcept {
      if (this != &other) {
        reset();
        id = std::exchange(other.id, 0);
      }
      return *this;
    }

    GLuint get() const noexcept { return id; }
    explicit operator bool() const noexcept { return id != 0; }
    void reset(GLuint handle = 0) noexcept {
      if (id) Traits::destroy(id);
      id = handle;
    }
  };

  struct GlBufferTraits      { static void destroy(GLuint id) noexcept { glDeleteBuffers(1, &id); } };
  struct GlVertexArrayTraits { static void destroy(GLuint id) noexcept { glDeleteVertexArrays(1, &id); } };
  struct GlShaderTraits      { static void destroy(GLuint id) noexcept { glDeleteShader(id); } };
  struct GlProgramTraits     { static void destroy(GLuint id) noexcept { glDeleteProgram(id); } };

  using GlBuffer = GlHandle<GlBufferTraits>;
  using GlVertexArray = GlHandle<GlVertexArrayTraits>;
  using GlShader = GlHandle<GlShaderTraits>;
  using GlProgram = GlHandle<GlProgramTraits>;

  inline constexpr std::size_t kQuadStartTop = 0;
  inline constexpr std::size_t kQuadStartBottom = 1;
  inline constexpr std::size_t kQuadEndTop = 2;
  inline constexpr std::size_t kQuadEndBottom = 3;
  inline constexpr std::size_t kQuadVertexCount = 4;

  inline constexpr std::array<GLuint, 6> kQuadIndices = {
    kQuadStartTop, kQuadStartBottom, kQuadEndBottom,
    kQuadStartTop, kQuadEndBottom, kQuadEndTop };

  inline std::array<glm::vec2, kQuadVertexCount> quadVertices(const ScreenQuad& quad) noexcept {
    std::array<glm::vec2, kQuadVertexCount> v;
    v[kQuadStartTop] = quad.startTop;
    v[kQuadStartBottom] = quad.startBottom;
    v[kQuadEndTop] = quad.endTop;
    v[kQuadEndBottom] = quad.endBottom;
    return v;
  }

  static_assert(kMaxBatchVertices == kMaxBatchQuads * kQuadVertexCount, "batch vertex limit");
  static_assert(kMaxBatchElements == kMaxBatchQuads * kQuadIndices.size(), "batch element limit");

  class RenderBatch {
    GlProgram program;
    GlVertexArray vao;
    GlBuffer vbo;
    GlBuffer ebo;
    std::vector<glm::vec2> staging;

    void batchCreate();
  public:
    RenderBatch() = default;
    ~RenderBatch() = default;

    RenderBatch(const RenderBatch&) = delete;
    RenderBatch& operator=(const RenderBatch&) = delete;
    RenderBatch(RenderBatch&&) noexcept = default;
    RenderBatch& operator=(RenderBatch&&) noexcept = default;

    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const std::vector<ScreenQuad>& quads);
    void release() noexcept;
  };
}
