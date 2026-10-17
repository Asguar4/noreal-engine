#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdexcept>

#include <glm/vec2.hpp>

#include "../platform.hpp"
#include "./iPlatform.hpp"

namespace riz {
  Key keyFromGlfw(int glfwKey) noexcept {
    if (glfwKey >= GLFW_KEY_A && glfwKey <= GLFW_KEY_Z) {
      return static_cast<Key>(static_cast<int>(Key::A) + (glfwKey - GLFW_KEY_A));
    }
    if (glfwKey >= GLFW_KEY_0 && glfwKey <= GLFW_KEY_9) {
      return static_cast<Key>(static_cast<int>(Key::Num0) + (glfwKey - GLFW_KEY_0));
    }
    switch (glfwKey) {
      case GLFW_KEY_SPACE:         return Key::Space;
      case GLFW_KEY_ESCAPE:        return Key::Escape;
      case GLFW_KEY_ENTER:         return Key::Enter;
      case GLFW_KEY_TAB:           return Key::Tab;
      case GLFW_KEY_BACKSPACE:     return Key::Backspace;
      case GLFW_KEY_LEFT:          return Key::Left;
      case GLFW_KEY_RIGHT:         return Key::Right;
      case GLFW_KEY_UP:            return Key::Up;
      case GLFW_KEY_DOWN:          return Key::Down;
      case GLFW_KEY_LEFT_SHIFT:    return Key::LeftShift;
      case GLFW_KEY_RIGHT_SHIFT:   return Key::RightShift;
      case GLFW_KEY_LEFT_CONTROL:  return Key::LeftControl;
      case GLFW_KEY_RIGHT_CONTROL: return Key::RightControl;
      case GLFW_KEY_LEFT_ALT:      return Key::LeftAlt;
      case GLFW_KEY_RIGHT_ALT:     return Key::RightAlt;
      default:                     return Key::Unknown;
    }
  }

  KeyAction keyActionFromGlfw(int glfwAction) noexcept {
    switch (glfwAction) {
      case GLFW_PRESS:  return KeyAction::Press;
      case GLFW_REPEAT: return KeyAction::Repeat;
      default:          return KeyAction::Release;
    }
  }
}

namespace {
  void onKeyCb(GLFWwindow* handle, int key, int, int action, int) {
    auto* self = static_cast<riz::Window*>(glfwGetWindowUserPointer(handle));
    if (self) self->input().onKey(riz::keyFromGlfw(key), riz::keyActionFromGlfw(action));
  }

  void onCursorCb(GLFWwindow* handle, double x, double y) {
    auto* self = static_cast<riz::Window*>(glfwGetWindowUserPointer(handle));
    if (self) self->input().onCursorPos(x, y);
  }
}

namespace riz {
  Window::Window(const WindowConfig& config) {
    if (glfwInit() != GLFW_TRUE) {
      throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, config.visible ? GLFW_TRUE : GLFW_FALSE);

    handle = glfwCreateWindow(config.width, config.height, config.title.c_str(), nullptr, nullptr);
    if (!handle) {
      glfwTerminate();
      throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(handle);
    if (config.requestVsync) {
      glfwSwapInterval(1);
    }
    glfwSetWindowUserPointer(handle, this);
    glfwSetKeyCallback(handle, onKeyCb);
    glfwSetCursorPosCallback(handle, onCursorCb);
    if (config.captureCursor) {
      glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }

  Window::~Window() {
    glfwDestroyWindow(handle);
    glfwTerminate();
  }

  bool Window::shouldClose() const {
    return glfwWindowShouldClose(handle) != 0;
  }

  void Window::requestClose() {
    glfwSetWindowShouldClose(handle, GLFW_TRUE);
  }

  void Window::pollEvents() {
    inputState.beginFrame();
    glfwPollEvents();
  }

  void Window::swapBuffers() {
    glfwSwapBuffers(handle);
  }

  glm::ivec2 Window::framebufferSize() const {
    int w = 0;
    int h = 0;
    glfwGetFramebufferSize(handle, &w, &h);
    return glm::ivec2{w, h};
  }

  Input& Window::input() noexcept {
    return inputState;
  }

  const Input& Window::input() const noexcept {
    return inputState;
  }
}
