#pragma once

#include <array>
#include <cstddef>
#include <functional>

#include <glm/vec2.hpp>

#include "./input.hpp"

namespace riz {
  inline constexpr double kDefaultStepSeconds = 1.0 / 60.0;
  inline constexpr double kDefaultMaxFrameSeconds = 0.25;
  inline constexpr int kDefaultMaxStepsPerFrame = 15;
  inline constexpr double kDefaultSnapToleranceSeconds = 0.001;
  inline constexpr int kDefaultFrameAverageCount = 8;
  inline constexpr int kMaxFrameAverageCount = 16;

  double steadyClockSeconds() noexcept;

  class FrameClock {
  public:
    explicit FrameClock(std::function<double()> now);
    double tick();

  private:
    std::function<double()> now;
    double last = 0.0;
    bool started = false;
  };

  class FixedTimestep {
  public:
    FixedTimestep();
    FixedTimestep(double stepSeconds, double maxFrameSeconds, int maxStepsPerFrame);

    int advance(double frameSeconds) noexcept;
    double alpha() const noexcept;

    double stepSeconds() const noexcept;
    double maxFrameSeconds() const noexcept;
    int maxStepsPerFrame() const noexcept;

  private:
    double step;
    double maxFrame;
    int maxSteps;
    double accumulator = 0.0;
  };

  class FrameTimeFilter {
  public:
    FrameTimeFilter();
    FrameTimeFilter(double stepSeconds, double maxFrameSeconds, double snapToleranceSeconds, int averageCount);

    double filter(double rawFrameSeconds) noexcept;

    double stepSeconds() const noexcept;
    double maxFrameSeconds() const noexcept;
    double snapToleranceSeconds() const noexcept;
    int averageCount() const noexcept;

  private:
    double step;
    double maxFrame;
    double tolerance;
    int count;
    std::array<double, kMaxFrameAverageCount> history{};
    int size = 0;
    int next = 0;
  };

  class StepInput {
  public:
    bool isDown(Key key) const noexcept;
    bool wasPressed(Key key) const noexcept;
    bool wasReleased(Key key) const noexcept;
    glm::vec2 mouseDelta() const noexcept;

  private:
    friend class InputLatch;
    static constexpr std::size_t kKeyCount = static_cast<std::size_t>(Key::Count);
    std::array<bool, kKeyCount> down{};
    std::array<bool, kKeyCount> pressed{};
    std::array<bool, kKeyCount> released{};
    glm::vec2 delta{0.f, 0.f};
  };

  class InputLatch {
  public:
    void capture(const Input& frame) noexcept;
    StepInput take() noexcept;

  private:
    StepInput pending;
  };
}
