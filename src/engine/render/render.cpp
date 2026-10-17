#include <stdexcept>
#include <string>

#include <GL/glew.h>
#include <glm/vec2.hpp>

#include "../render.hpp"
#include "../io.hpp"

namespace riz {
  Renderer::Renderer(glm::ivec2 framebufferSize, const std::string& pathVert, const std::string& pathFrag) {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      throw std::runtime_error(
        std::string("GLEW initialization failed: ") +
        reinterpret_cast<const char *>(glewGetErrorString(err)));
    }
    while (glGetError() != GL_NO_ERROR) {}

    glViewport(0, 0, framebufferSize.x, framebufferSize.y);

    batch.init(pathVert, pathFrag);
  }

  void Renderer::begin() {
    glClearColor(0.08f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void Renderer::draw(const std::vector<glm::vec2>& quadVertices) {
    batch.draw(quadVertices);
  }
}
