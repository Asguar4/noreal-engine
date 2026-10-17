#pragma once

#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "../render.hpp"

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

  class RenderBatch {
    GlProgram program;
    GlVertexArray vao;
    GlBuffer vbo;
    GlBuffer ebo;

    void batchCreate();
  public:
    RenderBatch() = default;
    ~RenderBatch() = default;

    RenderBatch(const RenderBatch&) = delete;
    RenderBatch& operator=(const RenderBatch&) = delete;
    RenderBatch(RenderBatch&&) noexcept = default;
    RenderBatch& operator=(RenderBatch&&) noexcept = default;

    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const std::vector<glm::vec2>& vertices);
    void release() noexcept;
  };
}
