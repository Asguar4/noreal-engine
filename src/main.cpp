#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>

#include "engine/render.hpp"

static bool w = false;
static bool a = false;
static bool s = false;
static bool d = false;
static bool o = false;
static bool p = false;

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

struct Entity {
  glm::vec2 pos;
  float angle;
  float fov;
};

struct Wall {
  glm::vec2 pos1;
  glm::vec2 pos2;
  float height;
};

int main() {
  riz::Renderer render("./shaders/default.vert", "./shaders/default.frag");

  Entity player = { {0, 0}, {0}, {30} };
  Wall wall { {3, 5}, {-3, 5}, {4} };

  float move = 0.005;

  while (!render.windowClose()) {
    render.begin();

    float dx1 = wall.pos1[0] - player.pos[0];
    float dz1 = wall.pos1[1] - player.pos[1];
    float dx2 = wall.pos2[0] - player.pos[0];
    float dz2 = wall.pos2[1] - player.pos[1];

    float wx1 = dx1 * cos(player.angle) - dz1 * sin(player.angle);
    float wz1 = dx1 * sin(player.angle) + dz1 * cos(player.angle);
    float wx2 = dx2 * cos(player.angle) - dz2 * sin(player.angle);
    float wz2 = dx2 * sin(player.angle) + dz2 * cos(player.angle);

    float x1 = wx1 / (wz1 * tan(player.fov / 2));
    float x2 = wx2 / (wz2 * tan(player.fov / 2));

    float y1 = wall.height / wz1;
    float y2 = wall.height / wz2;

    riz::Array<glm::vec2> vertices {
      glm::vec2{x1,  y1},
      glm::vec2{x1, -y1},
      glm::vec2{x2, -y2},
      glm::vec2{x2,  y2}
    };

    float mf = (w ? 1.f : 0.f) - (s ? 1.f : 0.f);
    float mr = (a ? 1.f : 0.f) - (d ? 1.f : 0.f);

    float dx = mf * sin(player.angle) + mr * cos(player.angle);
    float dy = mf * cos(player.angle) - mr * sin(player.angle);

    player.pos[0] += dx * move;
    player.pos[1] += dy * move;

    if (o) {
      player.angle += 0.001;
    }
    if (p) {
      player.angle -= 0.001;
    }

    render.update(vertices);

    render.end();
  }

  return 0;
}
