#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "./iRender.hpp"
#include "../array.hpp"
#include "../io.hpp"

static GLuint renderShaderCreate(const std::string& pathShader, GLenum type) {
  GLint success;
  GLchar log[512];

  riz::File fileShader(pathShader);
  if (!fileShader.valid()) {
    std::cerr << "Error reading shader: " << pathShader << std::endl;
    exit(1);
  }
  GLuint shader = glCreateShader(type);
  
  const char * sourceShader = fileShader.getData();
  glShaderSource(shader, 1, &sourceShader, nullptr);
  glCompileShader(shader);

  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, nullptr, log);
    std::cerr << "Error compiling shader: " << log << std::endl;
    exit(1);
  }

  return shader;
}

static GLuint shaderProgramCreate(const std::string& pathVert, const std::string& pathFrag) {
    GLint success;
    GLchar log[512];
    
    GLuint vertexShader = renderShaderCreate(pathVert, GL_VERTEX_SHADER);
    GLuint fragmentShader = renderShaderCreate(pathFrag, GL_FRAGMENT_SHADER);

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 512, nullptr, log);
      std::cerr << "Error linking shader: " << log << std::endl;
      exit(1);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shader;
}

namespace riz {
  void RenderLine::lineCreate() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
  }

  void RenderLine::init(const std::string& pathVert, const std::string& pathFrag) {
    shader = shaderProgramCreate(pathVert, pathFrag);
    lineCreate();
  }

  void RenderLine::draw(const riz::Array<glm::vec2>& vertices) {
    glUseProgram(shader);
    glLineWidth(3);

    float x = vertices[0][0] - vertices[1][0];
    float y = vertices[0][1] - vertices[1][1];

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 2, &vertices[0], GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, 2);

    glBindVertexArray(0);
  }

  void RenderBatch::batchCreate() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint indices[MAX_BATCH_ELEMENTS];
    for (GLuint i = 0, offset = 0; i < MAX_BATCH_ELEMENTS; i += 6, offset += 4) {
      indices[i + 0] = offset + 0;
      indices[i + 1] = offset + 1;
      indices[i + 2] = offset + 2;
      indices[i + 3] = offset + 2;
      indices[i + 4] = offset + 3;
      indices[i + 5] = offset + 0;
    }

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(glm::vec2), nullptr, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_BATCH_ELEMENTS * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void RenderBatch::init(const std::string& pathVert, const std::string& pathFrag) {
    shader = shaderProgramCreate(pathVert, pathFrag);
    batchCreate();
  }

  void RenderBatch::draw(const riz::Array<glm::vec2>& vertices) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);

    glUseProgram(shader);
    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, (vertices.size() >> 2) * 6, GL_UNSIGNED_INT, nullptr);
  }
}
