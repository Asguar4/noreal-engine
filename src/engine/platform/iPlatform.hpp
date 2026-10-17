#pragma once

#include "../input.hpp"

namespace riz {
  Key keyFromGlfw(int glfwKey) noexcept;
  KeyAction keyActionFromGlfw(int glfwAction) noexcept;
}
