#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "./iRender.hpp"
#include "../io.hpp"

namespace {
  std::string shaderInfoLog(GLuint shader) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len <= 1) return std::string();
    std::vector<GLchar> log(static_cast<std::size_t>(len));
    glGetShaderInfoLog(shader, len, nullptr, log.data());
    return std::string(log.data());
  }

  std::string programInfoLog(GLuint program) {
    GLint len = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    if (len <= 1) return std::string();
    std::vector<GLchar> log(static_cast<std::size_t>(len));
    glGetProgramInfoLog(program, len, nullptr, log.data());
    return std::string(log.data());
  }

  riz::GlShader renderShaderCreate(const std::string& pathShader, GLenum type) {
    riz::File fileShader(pathShader);

    riz::GlShader shader(glCreateShader(type));
    if (!shader) {
      throw std::runtime_error("glCreateShader failed for: " + pathShader);
    }

    const char * sourceShader = fileShader.getData();
    glShaderSource(shader.get(), 1, &sourceShader, nullptr);
    glCompileShader(shader.get());

    GLint success = 0;
    glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &success);
    if (!success) {
      throw std::runtime_error("Error compiling shader " + pathShader + ": " + shaderInfoLog(shader.get()));
    }

    return shader;
  }

  riz::GlProgram shaderProgramCreate(const std::string& pathVert, const std::string& pathFrag) {
    riz::GlShader vertexShader = renderShaderCreate(pathVert, GL_VERTEX_SHADER);
    riz::GlShader fragmentShader = renderShaderCreate(pathFrag, GL_FRAGMENT_SHADER);

    riz::GlProgram program(glCreateProgram());
    if (!program) {
      throw std::runtime_error("glCreateProgram failed");
    }
    glAttachShader(program.get(), vertexShader.get());
    glAttachShader(program.get(), fragmentShader.get());
    glLinkProgram(program.get());

    GLint success = 0;
    glGetProgramiv(program.get(), GL_LINK_STATUS, &success);
    if (!success) {
      throw std::runtime_error("Error linking shader program: " + programInfoLog(program.get()));
    }

    glDetachShader(program.get(), vertexShader.get());
    glDetachShader(program.get(), fragmentShader.get());

    return program;
  }
}

namespace riz {

  void RenderBatch::release() noexcept {
    ebo.reset();
    vbo.reset();
    vao.reset();
    program.reset();
  }

  void RenderBatch::batchCreate() {
    std::vector<GLuint> indices(kMaxBatchElements);
    for (std::size_t q = 0; q < kMaxBatchQuads; ++q) {
      for (std::size_t k = 0; k < kQuadIndices.size(); ++k) {
        indices[q * kQuadIndices.size() + k] = static_cast<GLuint>(q * kQuadVertexCount + kQuadIndices[k]);
      }
    }

    GLuint id = 0;

    glGenVertexArrays(1, &id);
    vao.reset(id);
    glBindVertexArray(vao.get());

    glGenBuffers(1, &id);
    vbo.reset(id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo.get());
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(kMaxBatchVertices * sizeof(glm::vec2)), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

    glGenBuffers(1, &id);
    ebo.reset(id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.get());
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(kMaxBatchElements * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void RenderBatch::init(const std::string& pathVert, const std::string& pathFrag) {
    release();
    program = shaderProgramCreate(pathVert, pathFrag);
    try {
      batchCreate();
    } catch (...) {
      release();
      throw;
    }
  }

  void RenderBatch::draw(const std::vector<ScreenQuad>& quads) {
    const std::size_t count = std::min(quads.size(), kMaxBatchQuads);
    if (count == 0) return;

    staging.resize(count * kQuadVertexCount);
    for (std::size_t i = 0; i < count; ++i) {
      const std::array<glm::vec2, kQuadVertexCount> v = quadVertices(quads[i]);
      std::copy(v.begin(), v.end(), staging.begin() + static_cast<std::ptrdiff_t>(i * kQuadVertexCount));
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo.get());
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * kQuadVertexCount * sizeof(glm::vec2)), staging.data());

    glUseProgram(program.get());
    glBindVertexArray(vao.get());

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(count * kQuadIndices.size()), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}
