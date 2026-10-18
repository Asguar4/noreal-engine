#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/vec2.hpp>

namespace riz {
  enum class Key : std::uint8_t {
    Unknown = 0,
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Escape, Enter, Tab, Backspace,
    Left, Right, Up, Down,
    LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt,
    Count
  };

  enum class KeyAction : std::uint8_t { Press, Repeat, Release };

  inline constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);

  constexpr bool isValidKey(Key key) noexcept {
    const auto k = static_cast<std::size_t>(key);
    return k > 0 && k < kKeyCount;
  }

  class Input {
  public:
    bool isDown(Key key) const noexcept;
    bool wasPressed(Key key) const noexcept;
    bool wasReleased(Key key) const noexcept;
    glm::vec2 mouseDelta() const noexcept;

    void beginFrame() noexcept;
    void onKey(Key key, KeyAction action) noexcept;
    void onCursorPos(double x, double y) noexcept;

  private:
    std::array<bool, kKeyCount> down{};
    std::array<bool, kKeyCount> pressed{};
    std::array<bool, kKeyCount> released{};
    double lastX = 0.0;
    double lastY = 0.0;
    bool hasCursor = false;
    glm::vec2 delta{0.f, 0.f};
  };
}
