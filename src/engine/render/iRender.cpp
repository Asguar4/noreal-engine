#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "./iRender.hpp"
#include "../../containers/vector.hpp"
#include "../io.hpp"

namespace {
  // Delete the GL object on scope exit unless the id was zeroed (ownership handed over).
  struct ShaderGuard {
    GLuint id;
    ~ShaderGuard() { if (id) glDeleteShader(id); }
  };

  struct ProgramGuard {
    GLuint id;
    ~ProgramGuard() { if (id) glDeleteProgram(id); }
  };

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

  GLuint renderShaderCreate(const std::string& pathShader, GLenum type) {
    riz::File fileShader(pathShader);
    if (!fileShader.valid()) {
      throw std::runtime_error("Error reading shader: " + pathShader);
    }

    ShaderGuard guard { glCreateShader(type) };
    if (!guard.id) {
      throw std::runtime_error("glCreateShader failed for: " + pathShader);
    }

    const char * sourceShader = fileShader.getData();
    glShaderSource(guard.id, 1, &sourceShader, nullptr);
    glCompileShader(guard.id);

    GLint success = 0;
    glGetShaderiv(guard.id, GL_COMPILE_STATUS, &success);
    if (!success) {
      throw std::runtime_error("Error compiling shader " + pathShader + ": " + shaderInfoLog(guard.id));
    }

    const GLuint result = guard.id;
    guard.id = 0;
    return result;
  }

  GLuint shaderProgramCreate(const std::string& pathVert, const std::string& pathFrag) {
    ShaderGuard vertexShader { renderShaderCreate(pathVert, GL_VERTEX_SHADER) };
    ShaderGuard fragmentShader { renderShaderCreate(pathFrag, GL_FRAGMENT_SHADER) };

    ProgramGuard program { glCreateProgram() };
    if (!program.id) {
      throw std::runtime_error("glCreateProgram failed");
    }
    glAttachShader(program.id, vertexShader.id);
    glAttachShader(program.id, fragmentShader.id);
    glLinkProgram(program.id);

    GLint success = 0;
    glGetProgramiv(program.id, GL_LINK_STATUS, &success);
    if (!success) {
      throw std::runtime_error("Error linking shader program: " + programInfoLog(program.id));
    }

    glDetachShader(program.id, vertexShader.id);
    glDetachShader(program.id, fragmentShader.id);

    const GLuint result = program.id;
    program.id = 0;
    return result;
  }
}

namespace riz {
  // ---- RenderLine ----

  RenderLine::~RenderLine() {
    release();
  }

  RenderLine::RenderLine(RenderLine&& other) noexcept
    : vao(other.vao), vbo(other.vbo), shader(other.shader) {
    other.vao = 0;
    other.vbo = 0;
    other.shader = 0;
  }

  RenderLine& RenderLine::operator=(RenderLine&& other) noexcept {
    if (this != &other) {
      release();
      vao = other.vao;
      vbo = other.vbo;
      shader = other.shader;
      other.vao = 0;
      other.vbo = 0;
      other.shader = 0;
    }
    return *this;
  }

  void RenderLine::release() noexcept {
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (shader) { glDeleteProgram(shader); shader = 0; }
  }

  void RenderLine::lineCreate() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
  }

  void RenderLine::init(const std::string& pathVert, const std::string& pathFrag) {
    release();
    shader = shaderProgramCreate(pathVert, pathFrag);
    lineCreate();
  }

  void RenderLine::draw(const Vector<glm::vec2>& vertices) {
    if (vertices.size() < 2) return;

    glUseProgram(shader);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec2) * 2, &vertices[0]);
    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  // ---- RenderBatch ----

  RenderBatch::~RenderBatch() {
    release();
  }

  RenderBatch::RenderBatch(RenderBatch&& other) noexcept
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo), shader(other.shader) {
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.shader = 0;
  }

  RenderBatch& RenderBatch::operator=(RenderBatch&& other) noexcept {
    if (this != &other) {
      release();
      vao = other.vao;
      vbo = other.vbo;
      ebo = other.ebo;
      shader = other.shader;
      other.vao = 0;
      other.vbo = 0;
      other.ebo = 0;
      other.shader = 0;
    }
    return *this;
  }

  void RenderBatch::release() noexcept {
    if (ebo) { glDeleteBuffers(1, &ebo); ebo = 0; }
    if (vbo) { glDeleteBuffers(1, &vbo); vbo = 0; }
    if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
    if (shader) { glDeleteProgram(shader); shader = 0; }
  }

  void RenderBatch::batchCreate() {
    std::vector<GLuint> indices(kMaxBatchElements);
    for (std::size_t i = 0, offset = 0; i + 6 <= kMaxBatchElements; i += 6, offset += 4) {
      indices[i + 0] = static_cast<GLuint>(offset + 0);
      indices[i + 1] = static_cast<GLuint>(offset + 1);
      indices[i + 2] = static_cast<GLuint>(offset + 3);
      indices[i + 3] = static_cast<GLuint>(offset + 0);
      indices[i + 4] = static_cast<GLuint>(offset + 3);
      indices[i + 5] = static_cast<GLuint>(offset + 2);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(kMaxBatchVertices * sizeof(glm::vec2)), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(kMaxBatchElements * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void RenderBatch::init(const std::string& pathVert, const std::string& pathFrag) {
    release();
    shader = shaderProgramCreate(pathVert, pathFrag);
    try {
      batchCreate();
    } catch (...) {
      release();
      throw;
    }
  }

  void RenderBatch::draw(const Vector<glm::vec2>& vertices) {
    const std::size_t maxVerts = (kMaxBatchVertices / 4) * 4;
    const std::size_t count = std::min(vertices.size(), maxVerts) / 4 * 4;
    if (count == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(count * sizeof(glm::vec2)), &vertices[0]);

    glUseProgram(shader);
    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>((count / 4) * 6), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}
