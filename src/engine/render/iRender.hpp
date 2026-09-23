#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "../array.hpp"

#define MAX_BATCH_VERTICES 40000
#define MAX_BATCH_ELEMENTS 60000

namespace riz {
  class RenderLine {
    GLuint vao;
    GLuint vbo;
    GLuint shader;

    void lineCreate();
  public:
    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const Array<glm::vec2>& vertices);
  };

  class RenderBatch {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shader;

    void batchCreate();
  public:
    void init(const std::string& pathVert, const std::string& pathFrag);
    void draw(const Array<glm::vec2>& vertices);
  };
}
