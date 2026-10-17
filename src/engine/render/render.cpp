#include <stdexcept>
#include <string>
#include <utility>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "../render.hpp"
#include "../io.hpp"

namespace riz {
  void Renderer::createWindow() {
    if (glfwInit() != GLFW_TRUE) {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
    if (!window) {
      throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
  }

  Renderer::Renderer(const std::string& pathVert, const std::string& pathFrag) {
    try {
      createWindow();

      glewExperimental = GL_TRUE;
      GLenum err = glewInit();
      if (err != GLEW_OK) {
        throw std::runtime_error(
          std::string("GLEW initialization failed: ") +
          reinterpret_cast<const char *>(glewGetErrorString(err)));
      }
      // glewInit may leave a harmless GL_INVALID_ENUM in core profile
      while (glGetError() != GL_NO_ERROR) {}

      glfwSetKeyCallback(window, keyCallback);
      glfwSetCursorPosCallback(window, mouseCallback);
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

      int fbWidth = 0;
      int fbHeight = 0;
      glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
      glViewport(0, 0, fbWidth, fbHeight);

      batch.init(pathVert, pathFrag);
    } catch (...) {
      batch.release();
      if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
      }
      glfwTerminate();
      throw;
    }
  }

  Renderer::~Renderer() {
    batch.release();
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void Renderer::begin() {
    glClearColor(0.08f, 0.1f, 0.1f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void Renderer::update(Vector<glm::vec2>& newVerts) {
    glfwPollEvents();
    vertices = newVerts;
  }

  void Renderer::end() {
    batch.draw(vertices);
    glfwSwapBuffers(window);
  }

  bool Renderer::windowClose() {
    return glfwWindowShouldClose(window);
  }
}
