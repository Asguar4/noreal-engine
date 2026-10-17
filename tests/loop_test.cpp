#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "engine/input.hpp"
#include "engine/loop.hpp"

namespace {
  using riz::FixedTimestep;
  using riz::FrameClock;
  using riz::FrameTimeFilter;
  using riz::Input;
  using riz::InputLatch;
  using riz::Key;
  using riz::KeyAction;
  using riz::StepInput;

  static_assert(noexcept(std::declval<FixedTimestep&>().advance(0.0)), "advance is noexcept");
  static_assert(noexcept(std::declval<const FixedTimestep&>().alpha()), "alpha is noexcept");
  static_assert(noexcept(std::declval<InputLatch&>().capture(std::declval<const Input&>())), "capture is noexcept");
  static_assert(noexcept(std::declval<InputLatch&>().take()), "take is noexcept");
  static_assert(noexcept(std::declval<const StepInput&>().isDown(Key::A)), "isDown is noexcept");
  static_assert(noexcept(std::declval<const StepInput&>().wasPressed(Key::A)), "wasPressed is noexcept");
  static_assert(noexcept(std::declval<const StepInput&>().wasReleased(Key::A)), "wasReleased is noexcept");
  static_assert(noexcept(std::declval<const StepInput&>().mouseDelta()), "mouseDelta is noexcept");
  static_assert(noexcept(std::declval<FrameTimeFilter&>().filter(0.0)), "filter is noexcept");
  static_assert(noexcept(std::declval<const FrameTimeFilter&>().stepSeconds()), "stepSeconds is noexcept");
  static_assert(noexcept(std::declval<const FrameTimeFilter&>().maxFrameSeconds()), "maxFrameSeconds is noexcept");
  static_assert(noexcept(std::declval<const FrameTimeFilter&>().snapToleranceSeconds()), "snapToleranceSeconds is noexcept");
  static_assert(noexcept(std::declval<const FrameTimeFilter&>().averageCount()), "averageCount is noexcept");

  constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();
  constexpr double kInf = std::numeric_limits<double>::infinity();

  void expectAllFalse(const StepInput& in) {
    for (int k = 0; k <= static_cast<int>(Key::Count); ++k) {
      const Key key = static_cast<Key>(k);
      EXPECT_FALSE(in.isDown(key)) << k;
      EXPECT_FALSE(in.wasPressed(key)) << k;
      EXPECT_FALSE(in.wasReleased(key)) << k;
    }
    EXPECT_EQ(in.mouseDelta().x, 0.f);
    EXPECT_EQ(in.mouseDelta().y, 0.f);
  }
}

TEST(FixedTimestep, Defaults) {
  const FixedTimestep t;
  EXPECT_EQ(t.stepSeconds(), 1.0 / 60.0);
  EXPECT_EQ(t.maxFrameSeconds(), 0.25);
  EXPECT_EQ(t.maxStepsPerFrame(), 15);
  EXPECT_EQ(t.alpha(), 0.0);
}

TEST(FixedTimestep, InvalidConfigThrows) {
  EXPECT_THROW(FixedTimestep(0.0, 1.0, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(-1.0, 1.0, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(kNaN, 1.0, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(kInf, kInf, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(0.5, 0.25, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(0.125, kNaN, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(0.125, kInf, 4), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(0.125, 1.0, 0), std::invalid_argument);
  EXPECT_THROW(FixedTimestep(0.125, 1.0, -1), std::invalid_argument);
}

TEST(FixedTimestep, ExactSixtyFpsOneStepPerFrame) {
  FixedTimestep t;
  for (int i = 0; i < 600; ++i) {
    ASSERT_EQ(t.advance(1.0 / 60.0), 1) << i;
    ASSERT_EQ(t.alpha(), 0.0) << i;
  }
}

TEST(FixedTimestep, AccumulatesPartialFrames) {
  FixedTimestep t(0.125, 1.0, 8);
  EXPECT_EQ(t.advance(0.0625), 0);
  EXPECT_EQ(t.alpha(), 0.5);
  EXPECT_EQ(t.advance(0.0625), 1);
  EXPECT_EQ(t.alpha(), 0.0);
  EXPECT_EQ(t.advance(0.3125), 2);
  EXPECT_EQ(t.alpha(), 0.5);
}

TEST(FixedTimestep, HighFrameRateZeroStepFrames) {
  FixedTimestep t(0.125, 1.0, 8);
  const int expectedSteps[4] = {0, 0, 0, 1};
  const double expectedAlpha[4] = {0.25, 0.5, 0.75, 0.0};
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(t.advance(0.03125), expectedSteps[i % 4]) << i;
    EXPECT_EQ(t.alpha(), expectedAlpha[i % 4]) << i;
  }
}

TEST(FixedTimestep, NonPositiveAndNaNAddNothing) {
  FixedTimestep t(0.125, 1.0, 8);
  EXPECT_EQ(t.advance(0.0625), 0);
  EXPECT_EQ(t.advance(0.0), 0);
  EXPECT_EQ(t.alpha(), 0.5);
  EXPECT_EQ(t.advance(-1.0), 0);
  EXPECT_EQ(t.alpha(), 0.5);
  EXPECT_EQ(t.advance(-0.0), 0);
  EXPECT_EQ(t.alpha(), 0.5);
  EXPECT_EQ(t.advance(kNaN), 0);
  EXPECT_EQ(t.alpha(), 0.5);
}

TEST(FixedTimestep, LongFrameClamped) {
  FixedTimestep t(0.125, 0.5, 8);
  EXPECT_EQ(t.advance(10.0), 4);
  EXPECT_EQ(t.alpha(), 0.0);
  EXPECT_EQ(t.advance(kInf), 4);
  EXPECT_EQ(t.alpha(), 0.0);
}

TEST(FixedTimestep, MaxStepsCapKeepsPhase) {
  FixedTimestep t(0.125, 2.0, 3);
  EXPECT_EQ(t.advance(1.0625), 3);
  EXPECT_EQ(t.alpha(), 0.5);
  EXPECT_EQ(t.advance(0.0), 0);
}

TEST(FixedTimestep, RandomFramesConserveTime) {
  FixedTimestep t;
  std::mt19937 rng(12345);
  std::uniform_real_distribution<double> dist(0.0, 0.1);
  long long totalSteps = 0;
  double totalTime = 0.0;
  for (int i = 0; i < 10000; ++i) {
    const double dt = dist(rng);
    const int n = t.advance(dt);
    ASSERT_GE(n, 0);
    ASSERT_LE(n, 15);
    ASSERT_GE(t.alpha(), 0.0);
    ASSERT_LT(t.alpha(), 1.0);
    totalSteps += n;
    totalTime += dt;
  }
  const double simulated = static_cast<double>(totalSteps) * t.stepSeconds() + t.alpha() * t.stepSeconds();
  EXPECT_LT(std::abs(simulated - totalTime), 1e-6);
}

TEST(FrameClock, FirstTickZeroThenDifferences) {
  const double values[5] = {5.0, 5.25, 5.25, 6.0, 5.5};
  int calls = 0;
  FrameClock clock([&]() { return values[calls++]; });
  EXPECT_EQ(calls, 0);
  EXPECT_EQ(clock.tick(), 0.0);
  EXPECT_EQ(clock.tick(), 0.25);
  EXPECT_EQ(clock.tick(), 0.0);
  EXPECT_EQ(clock.tick(), 0.75);
  EXPECT_EQ(clock.tick(), -0.5);
  EXPECT_EQ(calls, 5);
}

TEST(FrameClock, EmptyFunctionThrows) {
  EXPECT_THROW(FrameClock(std::function<double()>{}), std::invalid_argument);
}

TEST(FrameClock, SteadyClockMonotonic) {
  double prev = riz::steadyClockSeconds();
  for (int i = 0; i < 1000; ++i) {
    const double now = riz::steadyClockSeconds();
    ASSERT_GE(now, prev);
    prev = now;
  }
  FrameClock clock(riz::steadyClockSeconds);
  for (int i = 0; i < 100; ++i) EXPECT_GE(clock.tick(), 0.0);
}

TEST(InputLatch, DefaultEmpty) {
  expectAllFalse(StepInput{});
  InputLatch latch;
  expectAllFalse(latch.take());
}

TEST(InputLatch, EdgeToFirstStepOnly) {
  Input input;
  InputLatch latch;
  input.onKey(Key::W, KeyAction::Press);
  latch.capture(input);
  const StepInput a = latch.take();
  const StepInput b = latch.take();
  const StepInput c = latch.take();
  EXPECT_TRUE(a.wasPressed(Key::W));
  EXPECT_FALSE(b.wasPressed(Key::W));
  EXPECT_FALSE(c.wasPressed(Key::W));
  EXPECT_TRUE(a.isDown(Key::W));
  EXPECT_TRUE(b.isDown(Key::W));
  EXPECT_TRUE(c.isDown(Key::W));
}

TEST(InputLatch, EdgeKeptAcrossZeroStepFrames) {
  Input input;
  InputLatch latch;
  input.onKey(Key::Escape, KeyAction::Press);
  latch.capture(input);
  input.beginFrame();
  latch.capture(input);
  EXPECT_TRUE(latch.take().wasPressed(Key::Escape));
  EXPECT_FALSE(latch.take().wasPressed(Key::Escape));
}

TEST(InputLatch, MouseDeltaSummedUntilTaken) {
  Input input;
  InputLatch latch;
  input.onCursorPos(0.0, 0.0);
  input.beginFrame();
  input.onCursorPos(3.0, 0.0);
  latch.capture(input);
  input.beginFrame();
  input.onCursorPos(7.0, 1.0);
  latch.capture(input);
  const StepInput first = latch.take();
  EXPECT_EQ(first.mouseDelta().x, 7.f);
  EXPECT_EQ(first.mouseDelta().y, 1.f);
  const StepInput second = latch.take();
  EXPECT_EQ(second.mouseDelta().x, 0.f);
  EXPECT_EQ(second.mouseDelta().y, 0.f);
}

TEST(InputLatch, PressAndReleaseBeforeStep) {
  Input input;
  InputLatch latch;
  input.onKey(Key::A, KeyAction::Press);
  latch.capture(input);
  input.beginFrame();
  input.onKey(Key::A, KeyAction::Release);
  latch.capture(input);
  const StepInput in = latch.take();
  EXPECT_TRUE(in.wasPressed(Key::A));
  EXPECT_TRUE(in.wasReleased(Key::A));
  EXPECT_FALSE(in.isDown(Key::A));
}

TEST(InputLatch, InvalidKeysFalse) {
  Input input;
  InputLatch latch;
  for (int k = 1; k < static_cast<int>(Key::Count); ++k) input.onKey(static_cast<Key>(k), KeyAction::Press);
  latch.capture(input);
  const StepInput in = latch.take();
  const Key invalid[3] = {Key::Unknown, Key::Count, static_cast<Key>(200)};
  for (const Key key : invalid) {
    EXPECT_FALSE(in.isDown(key));
    EXPECT_FALSE(in.wasPressed(key));
    EXPECT_FALSE(in.wasReleased(key));
  }
  EXPECT_TRUE(in.isDown(Key::A));
  EXPECT_TRUE(in.wasPressed(Key::RightAlt));
}

TEST(GameLoopSim, EdgesNotLostOrDoubled) {
  FixedTimestep t(0.125, 1.0, 8);
  Input input;
  InputLatch latch;
  const double dts[6] = {0.0625, 0.0625, 0.375, 0.03125, 0.03125, 0.0625};
  const int expectedSteps[6] = {0, 1, 3, 0, 0, 1};
  std::vector<std::vector<bool>> pressed(6), released(6);
  bool downAtF5 = true;
  for (int f = 0; f < 6; ++f) {
    input.beginFrame();
    switch (f) {
      case 0: input.onKey(Key::Space, KeyAction::Press); break;
      case 1: input.onKey(Key::Space, KeyAction::Release); break;
      case 2: input.onKey(Key::Space, KeyAction::Press); break;
      case 3:
        input.onKey(Key::Space, KeyAction::Release);
        input.onKey(Key::Space, KeyAction::Press);
        break;
      case 5: input.onKey(Key::Space, KeyAction::Release); break;
      default: break;
    }
    latch.capture(input);
    const int n = t.advance(dts[f]);
    ASSERT_EQ(n, expectedSteps[f]) << f;
    for (int i = 0; i < n; ++i) {
      const StepInput in = latch.take();
      pressed[f].push_back(in.wasPressed(Key::Space));
      released[f].push_back(in.wasReleased(Key::Space));
      if (f == 5) downAtF5 = in.isDown(Key::Space);
    }
  }
  EXPECT_EQ(pressed[1], std::vector<bool>({true}));
  EXPECT_EQ(pressed[2], std::vector<bool>({true, false, false}));
  EXPECT_EQ(pressed[5], std::vector<bool>({true}));
  EXPECT_EQ(released[1], std::vector<bool>({true}));
  EXPECT_EQ(released[2], std::vector<bool>({false, false, false}));
  EXPECT_EQ(released[5], std::vector<bool>({true}));
  EXPECT_FALSE(downAtF5);
}

TEST(GameLoopSim, MouseTotalIndependentOfFrameRate) {
  const int rates[4] = {4, 8, 16, 32};
  for (const int n : rates) {
    FixedTimestep t(0.125, 1.0, 8);
    Input input;
    InputLatch latch;
    input.onCursorPos(0.0, 0.0);
    const double dt = 1.0 / n;
    const double dx = 64.0 / n;
    double x = 0.0;
    int totalSteps = 0;
    float totalDelta = 0.f;
    for (int f = 0; f < n; ++f) {
      input.beginFrame();
      x += dx;
      input.onCursorPos(x, 0.0);
      latch.capture(input);
      const int steps = t.advance(dt);
      for (int i = 0; i < steps; ++i) {
        totalDelta += latch.take().mouseDelta().x;
        ++totalSteps;
      }
    }
    EXPECT_EQ(totalSteps, 8) << n;
    EXPECT_EQ(totalDelta, 64.f) << n;
  }
}

TEST(FrameTimeFilter, Defaults) {
  const FrameTimeFilter f;
  EXPECT_EQ(f.stepSeconds(), 1.0 / 60.0);
  EXPECT_EQ(f.maxFrameSeconds(), 0.25);
  EXPECT_EQ(f.snapToleranceSeconds(), 0.001);
  EXPECT_EQ(f.averageCount(), 8);
}

TEST(FrameTimeFilter, InvalidConfigThrows) {
  EXPECT_THROW(FrameTimeFilter(0.0, 1.0, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(-1.0, 1.0, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(kNaN, 1.0, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(kInf, 1.0, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 0.0625, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, kNaN, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, kInf, 0.0, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, -0.001, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, kNaN, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, kInf, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, 0.0625, 4), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, 0.0, 0), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, 0.0, -1), std::invalid_argument);
  EXPECT_THROW(FrameTimeFilter(0.125, 1.0, 0.0, 17), std::invalid_argument);
}

TEST(FrameTimeFilter, NonPositiveAndNaNNotRecorded) {
  FrameTimeFilter f(0.125, 1.0, 0.0, 4);
  EXPECT_EQ(f.filter(0.0625), 0.0625);
  EXPECT_EQ(f.filter(0.0), 0.0);
  EXPECT_EQ(f.filter(-1.0), 0.0);
  EXPECT_EQ(f.filter(-0.0), 0.0);
  EXPECT_EQ(f.filter(kNaN), 0.0);
  EXPECT_EQ(f.filter(0.03125), 0.046875);
}

TEST(FrameTimeFilter, LongFrameClampedBeforeAveraging) {
  FrameTimeFilter f(0.125, 0.5, 0.0, 2);
  EXPECT_EQ(f.filter(10.0), 0.5);
  EXPECT_EQ(f.filter(kInf), 0.5);
  EXPECT_EQ(f.filter(0.0625), 0.28125);
}

TEST(FrameTimeFilter, MovingAverageWindow) {
  FrameTimeFilter f(0.125, 1.0, 0.0, 2);
  EXPECT_EQ(f.filter(0.0625), 0.0625);
  EXPECT_EQ(f.filter(0.03125), 0.046875);
  EXPECT_EQ(f.filter(0.25), 0.140625);
}

TEST(FrameTimeFilter, SnapsNearMultiples) {
  FrameTimeFilter f(0.125, 1.0, 0.0078125, 1);
  EXPECT_EQ(f.filter(0.130859375), 0.125);
  EXPECT_EQ(f.filter(0.119140625), 0.125);
  EXPECT_EQ(f.filter(0.2578125), 0.25);
  EXPECT_EQ(f.filter(0.13671875), 0.13671875);
  EXPECT_EQ(f.filter(0.0625), 0.0625);
  EXPECT_EQ(f.filter(0.0078125), 0.0078125);
}

TEST(GameLoopSim, AccumulatedSixtyHzClockOneStepPerFrame) {
  const int frames = 100000;
  {
    double t = 0.0;
    FrameClock clock([&t]() {
      const double now = t;
      t += 1.0 / 60.0;
      return now;
    });
    FixedTimestep timestep;
    FrameTimeFilter filter;
    for (int f = 0; f < frames; ++f) {
      const int n = timestep.advance(filter.filter(clock.tick()));
      ASSERT_EQ(n, f == 0 ? 0 : 1) << f;
    }
  }
  double t = 0.0;
  FrameClock clock([&t]() {
    const double now = t;
    t += 1.0 / 60.0;
    return now;
  });
  FixedTimestep timestep;
  int deviating = 0;
  for (int f = 0; f < frames; ++f) {
    const int n = timestep.advance(clock.tick());
    if (f >= 1 && n != 1) ++deviating;
  }
  EXPECT_GE(deviating, 1);
}

TEST(GameLoopSim, JitteredNearSixtyHzLocksToOneStep) {
  const int frames = 100000;
  std::mt19937 rng(7);
  std::vector<double> stamps;
  stamps.reserve(frames + 1);
  for (int i = 0; i <= frames; ++i) {
    stamps.push_back(i / 59.94 + (static_cast<double>(rng()) / 4294967295.0 * 2.0 - 1.0) * 0.002);
  }
  {
    std::size_t idx = 0;
    FrameClock clock([&]() { return stamps[idx++]; });
    FixedTimestep timestep;
    FrameTimeFilter filter;
    for (int f = 0; f < frames; ++f) {
      const int n = timestep.advance(filter.filter(clock.tick()));
      if (f >= 9) ASSERT_EQ(n, 1) << f;
    }
  }
  std::size_t idx = 0;
  FrameClock clock([&]() { return stamps[idx++]; });
  FixedTimestep timestep;
  int deviating = 0;
  for (int f = 0; f < frames; ++f) {
    const int n = timestep.advance(clock.tick());
    if (f >= 9 && n != 1) ++deviating;
  }
  EXPECT_GT(deviating, 100);
}

TEST(GameLoopSim, HighRefreshTimeConserved) {
  const int frames = 100000;
  std::mt19937 rng(7);
  std::vector<double> stamps;
  stamps.reserve(frames);
  for (int i = 0; i < frames; ++i) {
    stamps.push_back(i / 144.0 + (static_cast<double>(rng()) / 4294967295.0 * 2.0 - 1.0) * 0.0005);
  }
  std::size_t idx = 0;
  FrameClock clock([&]() { return stamps[idx++]; });
  FixedTimestep timestep;
  FrameTimeFilter filter;
  long long totalSteps = 0;
  for (int f = 0; f < frames; ++f) {
    totalSteps += timestep.advance(filter.filter(clock.tick()));
  }
  const double simulated = static_cast<double>(totalSteps) * timestep.stepSeconds() + timestep.alpha() * timestep.stepSeconds();
  EXPECT_LT(std::abs(simulated - (stamps[frames - 1] - stamps[0])), 0.1);
}

TEST(GameLoopSim, LongFrameCatchUpBounded) {
  double t = 0.0;
  int calls = 0;
  FrameClock clock([&]() {
    const double now = t + (calls >= 100 ? 10.0 : 0.0);
    ++calls;
    t += 1.0 / 60.0;
    return now;
  });
  FixedTimestep timestep;
  FrameTimeFilter filter;
  std::vector<int> steps;
  for (int f = 0; f < 200; ++f) steps.push_back(timestep.advance(filter.filter(clock.tick())));
  EXPECT_EQ(steps[0], 0);
  for (int f = 1; f < 100; ++f) EXPECT_EQ(steps[f], 1) << f;
  int sum = 0;
  for (int f = 100; f < 120; ++f) sum += steps[f];
  EXPECT_GE(sum, 33);
  EXPECT_LE(sum, 35);
  for (int f = 100; f < 108; ++f) EXPECT_LE(steps[f], 3) << f;
  for (int f = 108; f < 200; ++f) EXPECT_EQ(steps[f], 1) << f;
}
