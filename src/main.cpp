#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "engine/render.hpp"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
}

int main() {
  riz::Renderer render("./shaders/default.vert", "./shaders/default.frag");

  riz::Array<glm::vec2> vertices {
    glm::vec2{ 0.5,  0.5},
    glm::vec2{ 0.5, -0.5},
    glm::vec2{-0.5, -0.5},
    glm::vec2{-0.5,  0.5}
  };

  while (!render.windowClose()) {
    render.begin();
 
    render.update(vertices);

    render.end();
  }

  return 0;
}
