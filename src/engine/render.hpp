#pragma once

#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "../containers/array.hpp"
#include "./render/iRender.hpp"

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);

namespace riz {
  class Renderer {
    Vector<glm::vec2> vertices;
    RenderBatch batch;
    GLFWwindow * window = nullptr;

    void createWindow();
  public:
    Renderer(const std::string& pathVert, const std::string& pathFrag);

    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void begin();
    void update(Vector<glm::vec2>& newVert);
    void end();

    bool windowClose();
  };
}
