#pragma once

#include <string>

#include <glm/vec2.hpp>

#include "./input.hpp"

struct GLFWwindow;   // global-namespace forward declaration, compatible with GLFW's `typedef struct GLFWwindow GLFWwindow;`

namespace riz {
  struct WindowConfig {
    int width = 800;
    int height = 600;
    std::string title = "OpenGL";
    bool resizable = false;
    bool visible = true;          // tests use false (hidden-window GL tests)
    bool captureCursor = true;    // GLFW_CURSOR_DISABLED (mouse-look)
  };

  // Owns GLFW (init/terminate), one window with its OpenGL 3.3 core context, and the event pump.
  // At most one live Window per process. Construct-destroy-construct in sequence is supported.
  // Not copyable or movable, because GLFW's window user pointer refers to this object. Main thread only.
  // Declare the Window before any object that owns GL resources of its context (for example riz::Renderer),
  // so those are destroyed first.
  class Window {
  public:
    explicit Window(const WindowConfig& config);   // throws std::runtime_error. Nothing stays alive after a throw
    ~Window();                                      // glfwDestroyWindow, then glfwTerminate
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool shouldClose() const;
    void requestClose();                    // glfwSetWindowShouldClose(handle, GLFW_TRUE)
    void pollEvents();                      // inputState.beginFrame(); glfwPollEvents();
    void swapBuffers();                     // glfwSwapBuffers
    glm::ivec2 framebufferSize() const;     // glfwGetFramebufferSize

    Input& input() noexcept;
    const Input& input() const noexcept;

  private:
    GLFWwindow * handle = nullptr;
    Input inputState;
  };
}
