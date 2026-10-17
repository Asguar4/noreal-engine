#include "../input.hpp"

namespace {
  // Valid, real keys are strictly between Unknown and Count.
  bool valid(riz::Key key) noexcept {
    const auto k = static_cast<std::size_t>(key);
    return k > 0 && k < static_cast<std::size_t>(riz::Key::Count);
  }
}

namespace riz {
  bool Input::isDown(Key key) const noexcept {
    return valid(key) && down[static_cast<std::size_t>(key)];
  }

  bool Input::wasPressed(Key key) const noexcept {
    return valid(key) && pressed[static_cast<std::size_t>(key)];
  }

  bool Input::wasReleased(Key key) const noexcept {
    return valid(key) && released[static_cast<std::size_t>(key)];
  }

  glm::vec2 Input::mouseDelta() const noexcept {
    return delta;
  }

  void Input::beginFrame() noexcept {
    pressed.fill(false);
    released.fill(false);
    delta = glm::vec2{0.f, 0.f};
  }

  void Input::onKey(Key key, KeyAction action) noexcept {
    if (!valid(key)) return;
    const auto i = static_cast<std::size_t>(key);
    switch (action) {
      case KeyAction::Press:
        down[i] = true;
        pressed[i] = true;
        break;
      case KeyAction::Repeat:
        down[i] = true;
        break;
      case KeyAction::Release:
        down[i] = false;
        released[i] = true;
        break;
    }
  }

  void Input::onCursorPos(double x, double y) noexcept {
    if (!hasCursor) {
      lastX = x;
      lastY = y;
      hasCursor = true;
    }
    delta.x += static_cast<float>(x - lastX);
    delta.y += static_cast<float>(y - lastY);
    lastX = x;
    lastY = y;
  }
}
