#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <glm/common.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>

#include "engine/camera.hpp"
#include "engine/input.hpp"
#include "engine/loop.hpp"
#include "engine/movement.hpp"
#include "engine/platform.hpp"
#include "engine/render.hpp"
#include "engine/wallQuad.hpp"

namespace {
  constexpr float kFovDegrees = 30.f;
  constexpr float kStepSeconds = static_cast<float>(riz::kDefaultStepSeconds);
  constexpr float kLegacyMovePerFrame = 0.005f;
  constexpr float kMoveSpeed = kLegacyMovePerFrame * 60.f;
  constexpr float kMouseSensitivity = 0.002f;
  static_assert(kMoveSpeed * kStepSeconds == kLegacyMovePerFrame, "one 1/60 s step moves exactly one old frame");

  constexpr long kMaxSmokeFrames = 100000;

  bool parseSmokeFrames(const char* text, std::size_t& out) {
    if (!text || !*text) return false;
    for (const char* c = text; *c; ++c) {
      if (*c < '0' || *c > '9') return false;
    }
    if (std::string(text).size() > 6) return false;
    const long value = std::strtol(text, nullptr, 10);
    if (value < 1 || value > kMaxSmokeFrames) return false;
    out = static_cast<std::size_t>(value);
    return true;
  }
}

int main(int argc, char* argv[]) {
  std::size_t smokeFrames = 0;
  riz::WindowConfig config;
  config.requestVsync = true;
  if (argc >= 2 && std::string(argv[1]) == "--smoke-frames") {
    if (argc < 3 || !parseSmokeFrames(argv[2], smokeFrames)) {
      std::cerr << "usage: game [--smoke-frames N]" << std::endl;
      return EXIT_FAILURE;
    }
    config.visible = false;
    config.captureCursor = false;
    config.requestVsync = false;
  }

  try {
    riz::Window window(config);
    riz::Renderer render(window.framebufferSize(), "./shaders/default.vert", "./shaders/default.frag");
    const riz::Input& input = window.input();

    std::vector<glm::vec3> walls {
      glm::vec3 { 3, 5, 4},
      glm::vec3 {-3, 5, 4}
    };
    riz::Camera sim(glm::radians(kFovDegrees));
    riz::Camera view(glm::radians(kFovDegrees));
    glm::vec2 prevPos = sim.getPosition();
    float prevAngle = sim.getAngle();
    std::vector<riz::WallQuad> quads;

    riz::FrameClock clock(riz::steadyClockSeconds);
    riz::FixedTimestep timestep;
    riz::FrameTimeFilter frameFilter;
    riz::InputLatch latch;

    std::size_t frame = 0;

    while (!window.shouldClose() && (smokeFrames == 0 || frame < smokeFrames)) {
      window.pollEvents();
      latch.capture(input);

      const int steps = timestep.advance(frameFilter.filter(clock.tick()));
      for (int i = 0; i < steps; ++i) {
        const riz::StepInput in = latch.take();
        if (in.wasPressed(riz::Key::Escape)) window.requestClose();

        prevPos = sim.getPosition();
        prevAngle = sim.getAngle();

        float mf = (in.isDown(riz::Key::W) ? 1.f : 0.f) - (in.isDown(riz::Key::S) ? 1.f : 0.f);
        float mr = (in.isDown(riz::Key::D) ? 1.f : 0.f) - (in.isDown(riz::Key::A) ? 1.f : 0.f);

        const glm::vec2 move = riz::moveDirection(mf, mr, sim.forward(), sim.right()) * (kMoveSpeed * kStepSeconds);
        sim.setPosition(sim.getPosition() + move);
        sim.setAngle(sim.getAngle() + in.mouseDelta().x * kMouseSensitivity);
      }

      const float alpha = static_cast<float>(timestep.alpha());
      view.setPosition(glm::mix(prevPos, sim.getPosition(), alpha));
      view.setAngle(glm::mix(prevAngle, sim.getAngle(), alpha));

      render.begin();
      view.worldToScreen(walls, quads);
      render.draw(quads);
      window.swapBuffers();
      ++frame;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
