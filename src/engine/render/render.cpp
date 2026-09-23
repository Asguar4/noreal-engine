#include <iostream>
#include <string>
#include <utility>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp> 

#include "../render.hpp"
#include "../io.hpp"

namespace riz {
  void Renderer::createWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(800, 600, "OpenGL", nullptr, nullptr);
    if (!window) {
      std::cerr << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      exit(1);
    }

    glfwMakeContextCurrent(window);
  }

  Renderer::Renderer(const std::string& pathVert, const std::string& pathFrag) {
    createWindow();

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if (err != GLEW_OK) {
      std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << std::endl;
      glfwTerminate();
      exit(1);
    }

    glfwSetKeyCallback(window, keyCallback);
    //glfwSetCursorPosCallback(window, mouseCallback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glViewport(0, 0, 800, 600);

    batch.init(pathVert, pathFrag);
  }

  Renderer::~Renderer() {
    glfwTerminate();
  }
  
  void Renderer::begin() {
    glClearColor(0.08, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
  }

  void Renderer::update(Array<glm::vec2>& newVerts) {
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
