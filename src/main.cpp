#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/render.hpp"
#include "engine/entity.hpp"

static bool w = false;
static bool a = false;
static bool s = false;
static bool d = false;
static bool o = false;
static bool p = false;

static float lastX = 0.0;
static float lastY = 0.0;
static float mousedx = 0.0;
static float mousedy = 0.0;
static bool firstMouse = true;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
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

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  mousedx = xpos - lastX;
  mousedy = lastY - ypos;

  lastX = xpos;
  lastY = ypos;
}

struct Entity {
  glm::vec2 pos;
  float angle;
  float fov;
};

int main() {
  riz::Renderer render("./shaders/default.vert", "./shaders/default.frag");

  Entity player = { {0, 0}, {0}, {30} };
  riz::Array<glm::vec3> walls {
    glm::vec3 { 3, 5, 4},
    glm::vec3 {-3, 5, 4}
  };
  riz::Camera cum(30);

  float move = 0.005;

  while (!render.windowClose()) {
    render.begin();

    riz::Array<glm::vec2> vertices = cum.worldToScreen(walls);

    float mf = (w ? 1.f : 0.f) - (s ? 1.f : 0.f);
    float mr = (a ? 1.f : 0.f) - (d ? 1.f : 0.f);

    float dx = mf * sin(player.angle) + mr * cos(player.angle);
    float dy = mf * cos(player.angle) - mr * sin(player.angle);

    player.pos[0] += dx * move;
    player.pos[1] += dy * move;

    player.angle -= mousedx * 0.002;
    mousedx = 0.0;

    cum.setPosition(player.pos);
    cum.setAngle(player.angle);

    render.update(vertices);

    render.end();
  }

  return 0;
}
