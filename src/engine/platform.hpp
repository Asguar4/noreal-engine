#pragma once

#include <string>

#include <glm/vec2.hpp>

#include "./input.hpp"

struct GLFWwindow;

namespace riz {
  struct WindowConfig {
    int width = 800;
    int height = 600;
    std::string title = "OpenGL";
    bool resizable = false;
    bool visible = true;
    bool captureCursor = true;
  };

  class Window {
  public:
    explicit Window(const WindowConfig& config);
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool shouldClose() const;
    void requestClose();
    void pollEvents();
    void swapBuffers();
    glm::ivec2 framebufferSize() const;

    Input& input() noexcept;
    const Input& input() const noexcept;

  private:
    GLFWwindow * handle = nullptr;
    Input inputState;
  };
}
