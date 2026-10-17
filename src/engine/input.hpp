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
    Count                                         // sentinel, not a key
  };

  enum class KeyAction : std::uint8_t { Press, Repeat, Release };

  // Per-frame keyboard/mouse state. Engine-level and GLFW-free: riz::Window feeds it, and tests may feed it directly.
  // Frame semantics: Window::pollEvents() calls beginFrame() right before the OS events are dispatched, so the
  // "this frame" queries describe the events delivered by the most recent pollEvents().
  // Any Key value >= Key::Count behaves like Key::Count (ignored, queries return false).
  class Input {
  public:
    // ---- queries (application) ----
    bool isDown(Key key) const noexcept;        // Press/Repeat -> true, Release -> false. Unknown/Count -> false
    bool wasPressed(Key key) const noexcept;    // >= 1 Press event since the last beginFrame(). Latched: a press+release
                                                // inside one poll still reports true. Repeat does NOT count as pressed
    bool wasReleased(Key key) const noexcept;   // >= 1 Release event since the last beginFrame()
    glm::vec2 mouseDelta() const noexcept;      // cursor motion summed since the last beginFrame(), in screen coordinates
                                                // (+x right, +y DOWN). The very first cursor event ever contributes (0,0)

    // ---- feeding (platform layer and tests) ----
    void beginFrame() noexcept;                 // clears the pressed/released latches and mouseDelta. Keeps down state and the last cursor pos
    void onKey(Key key, KeyAction action) noexcept;   // Unknown/Count ignored
    void onCursorPos(double x, double y) noexcept;

  private:
    static constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);
    std::array<bool, kKeyCount> down{};
    std::array<bool, kKeyCount> pressed{};
    std::array<bool, kKeyCount> released{};
    double lastX = 0.0;
    double lastY = 0.0;
    bool hasCursor = false;                     // replaces main.cpp's firstMouse
    glm::vec2 delta{0.f, 0.f};
  };
}
