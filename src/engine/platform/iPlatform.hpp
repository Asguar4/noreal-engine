#pragma once

#include "../input.hpp"

namespace riz {
  // GLFW int codes -> engine enums. No GLFW include, so these are unit-testable.
  Key keyFromGlfw(int glfwKey) noexcept;                // unmapped -> Key::Unknown
  KeyAction keyActionFromGlfw(int glfwAction) noexcept; // GLFW_PRESS->Press, GLFW_REPEAT->Repeat, anything else->Release
}
