#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/trigonometric.hpp>

#include "engine/camera.hpp"
#include "engine/input.hpp"
#include "engine/platform.hpp"
#include "engine/render.hpp"

namespace {
  constexpr float kFovDegrees = 30.f;
  constexpr float kMoveSpeed = 0.005f;
  constexpr float kMouseSensitivity = 0.002f;

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
  if (argc >= 2 && std::string(argv[1]) == "--smoke-frames") {
    if (argc < 3 || !parseSmokeFrames(argv[2], smokeFrames)) {
      std::cerr << "usage: game [--smoke-frames N]" << std::endl;
      return EXIT_FAILURE;
    }
    config.visible = false;
    config.captureCursor = false;
  }

  try {
    riz::Window window(config);
    riz::Renderer render(window.framebufferSize(), "./shaders/default.vert", "./shaders/default.frag");
    const riz::Input& input = window.input();

    std::vector<glm::vec3> walls {
      glm::vec3 { 3, 5, 4},
      glm::vec3 {-3, 5, 4}
    };
    riz::Camera cam(glm::radians(kFovDegrees));
    std::vector<glm::vec2> vertices;

    std::size_t frame = 0;

    while (!window.shouldClose() && (smokeFrames == 0 || frame < smokeFrames)) {
      render.begin();

      float mf = (input.isDown(riz::Key::W) ? 1.f : 0.f) - (input.isDown(riz::Key::S) ? 1.f : 0.f);
      float mr = (input.isDown(riz::Key::D) ? 1.f : 0.f) - (input.isDown(riz::Key::A) ? 1.f : 0.f);

      const glm::vec2 step = (mf * cam.forward() + mr * cam.right()) * kMoveSpeed;
      cam.setPosition(cam.getPosition() + step);
      cam.setAngle(cam.getAngle() + input.mouseDelta().x * kMouseSensitivity);

      cam.worldToScreen(walls, vertices);

      window.pollEvents();
      if (input.wasPressed(riz::Key::Escape)) window.requestClose();

      render.draw(vertices);
      window.swapBuffers();
      ++frame;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
