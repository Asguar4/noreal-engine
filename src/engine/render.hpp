#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "./array.hpp"
#include "./render/iRender.hpp"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

namespace riz {
  class Renderer {
    Array<glm::vec2> vertices;
    RenderBatch batch;
    GLFWwindow * window;

    void createWindow();
  public:
    Renderer(const std::string& pathVert, const std::string& pathFrag);

    ~Renderer();

    void begin();
    void update(Array<glm::vec2>& newVert);
    void end();

    bool windowClose();
  };
}
