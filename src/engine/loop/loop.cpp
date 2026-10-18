#include "../loop.hpp"

#include <chrono>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace riz {
  double steadyClockSeconds() noexcept {
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
  }

  FrameClock::FrameClock(std::function<double()> nowFn) : now(std::move(nowFn)) {
    if (!now) throw std::invalid_argument("FrameClock: empty time source");
  }

  double FrameClock::tick() {
    const double t = now();
    if (!started) {
      started = true;
      last = t;
      return 0.0;
    }
    const double dt = t - last;
    last = t;
    return dt;
  }

  FixedTimestep::FixedTimestep()
    : FixedTimestep(kDefaultStepSeconds, kDefaultMaxFrameSeconds, kDefaultMaxStepsPerFrame) {}

  FixedTimestep::FixedTimestep(double stepSeconds, double maxFrameSeconds, int maxStepsPerFrame)
    : step(stepSeconds), maxFrame(maxFrameSeconds), maxSteps(maxStepsPerFrame) {
    if (!(std::isfinite(step) && step > 0.0)) throw std::invalid_argument("FixedTimestep: step must be finite and > 0");
    if (!(std::isfinite(maxFrame) && maxFrame >= step)) throw std::invalid_argument("FixedTimestep: maxFrame must be finite and >= step");
    if (maxSteps < 1) throw std::invalid_argument("FixedTimestep: maxSteps must be >= 1");
  }

  int FixedTimestep::advance(double frameSeconds) noexcept {
    if (!(frameSeconds > 0.0)) frameSeconds = 0.0;
    if (frameSeconds > maxFrame) frameSeconds = maxFrame;
    accumulator += frameSeconds;
    int n = 0;
    while (n < maxSteps && accumulator >= step) {
      accumulator -= step;
      ++n;
    }
    if (accumulator >= step) accumulator = std::fmod(accumulator, step);
    return n;
  }

  double FixedTimestep::alpha() const noexcept {
    return accumulator / step;
  }

  double FixedTimestep::stepSeconds() const noexcept {
    return step;
  }

  double FixedTimestep::maxFrameSeconds() const noexcept {
    return maxFrame;
  }

  int FixedTimestep::maxStepsPerFrame() const noexcept {
    return maxSteps;
  }

  FrameTimeFilter::FrameTimeFilter()
    : FrameTimeFilter(kDefaultStepSeconds, kDefaultMaxFrameSeconds, kDefaultSnapToleranceSeconds, kDefaultFrameAverageCount) {}

  FrameTimeFilter::FrameTimeFilter(double stepSeconds, double maxFrameSeconds, double snapToleranceSeconds, int averageCount)
    : step(stepSeconds), maxFrame(maxFrameSeconds), tolerance(snapToleranceSeconds), count(averageCount) {
    if (!(std::isfinite(step) && step > 0.0)) throw std::invalid_argument("FrameTimeFilter: step must be finite and > 0");
    if (!(std::isfinite(maxFrame) && maxFrame >= step)) throw std::invalid_argument("FrameTimeFilter: maxFrame must be finite and >= step");
    if (!(std::isfinite(tolerance) && tolerance >= 0.0 && tolerance < step / 2.0)) throw std::invalid_argument("FrameTimeFilter: tolerance must be finite, >= 0 and < step / 2");
    if (count < 1 || count > kMaxFrameAverageCount) throw std::invalid_argument("FrameTimeFilter: averageCount out of range");
  }

  double FrameTimeFilter::filter(double rawFrameSeconds) noexcept {
    if (!(rawFrameSeconds > 0.0)) return 0.0;
    if (rawFrameSeconds > maxFrame) rawFrameSeconds = maxFrame;
    history[static_cast<std::size_t>(next)] = rawFrameSeconds;
    next = (next + 1) % count;
    if (size < count) ++size;
    double sum = 0.0;
    for (int i = 0; i < size; ++i) sum += history[static_cast<std::size_t>(i)];
    const double avg = sum / size;
    const double k = std::floor(avg / step + 0.5);
    if (k >= 1.0 && std::abs(avg - k * step) <= tolerance) return k * step;
    return avg;
  }

  double FrameTimeFilter::stepSeconds() const noexcept {
    return step;
  }

  double FrameTimeFilter::maxFrameSeconds() const noexcept {
    return maxFrame;
  }

  double FrameTimeFilter::snapToleranceSeconds() const noexcept {
    return tolerance;
  }

  int FrameTimeFilter::averageCount() const noexcept {
    return count;
  }

  bool StepInput::isDown(Key key) const noexcept {
    return isValidKey(key) && down[static_cast<std::size_t>(key)];
  }

  bool StepInput::wasPressed(Key key) const noexcept {
    return isValidKey(key) && pressed[static_cast<std::size_t>(key)];
  }

  bool StepInput::wasReleased(Key key) const noexcept {
    return isValidKey(key) && released[static_cast<std::size_t>(key)];
  }

  glm::vec2 StepInput::mouseDelta() const noexcept {
    return delta;
  }

  void InputLatch::capture(const Input& frame) noexcept {
    for (std::size_t i = 1; i < kKeyCount; ++i) {
      const Key k = static_cast<Key>(i);
      pending.down[i] = frame.isDown(k);
      pending.pressed[i] = pending.pressed[i] || frame.wasPressed(k);
      pending.released[i] = pending.released[i] || frame.wasReleased(k);
    }
    pending.delta += frame.mouseDelta();
  }

  StepInput InputLatch::take() noexcept {
    StepInput out = pending;
    pending.pressed.fill(false);
    pending.released.fill(false);
    pending.delta = glm::vec2{0.f, 0.f};
    return out;
  }
}
