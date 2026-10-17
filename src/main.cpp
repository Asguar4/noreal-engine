#include <iostream>
#include <cmath>
#include <cstdlib>
#include <exception>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>

#include "engine/render.hpp"
#include "engine/entity.hpp"

static bool w = false;
static bool a = false;
static bool s = false;
static bool d = false;
static bool o = false;
static bool p = false;

static double lastX = 0.0;
static double lastY = 0.0;
static float mousedx = 0.f;
static float mousedy = 0.f;
static bool firstMouse = true;

void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mode*/) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }
  switch (key) {
    case GLFW_KEY_W:
      w = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_A:
      a = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_S:
      s = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_D:
      d = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_O:
      o = action != GLFW_RELEASE;
      break;
    case GLFW_KEY_P:
      p = action != GLFW_RELEASE;
      break;
    default:
      break;
  }
}

void mouseCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  mousedx += static_cast<float>(xpos - lastX);
  mousedy += static_cast<float>(lastY - ypos);

  lastX = xpos;
  lastY = ypos;
}

struct Entity {
  glm::vec2 pos;
  float angle;
  float fov;
};

int main() {
  try {
    riz::Renderer render("./shaders/default.vert", "./shaders/default.frag");

    Entity player = { {0, 0}, 0, 30 };
    riz::Vector<glm::vec3> walls {
      glm::vec3 { 3, 5, 4},
      glm::vec3 {-3, 5, 4}
    };
    riz::Camera cam(glm::radians(player.fov));

    const float move = 0.005f;

    while (!render.windowClose()) {
      render.begin();

      float mf = (w ? 1.f : 0.f) - (s ? 1.f : 0.f);
      float mr = (d ? 1.f : 0.f) - (a ? 1.f : 0.f);

      float dx = mf * std::sin(player.angle) + mr * std::cos(player.angle);
      float dy = mf * std::cos(player.angle) - mr * std::sin(player.angle);

      player.pos[0] += dx * move;
      player.pos[1] += dy * move;

      player.angle += mousedx * 0.002f;
      mousedx = 0.f;

      cam.setPosition(player.pos);
      cam.setAngle(player.angle);

      riz::Vector<glm::vec2> vertices = cam.worldToScreen(walls);

      render.update(vertices);

      render.end();
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
