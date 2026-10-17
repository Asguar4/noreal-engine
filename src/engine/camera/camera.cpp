#include <cmath>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

#include "../camera.hpp"

namespace {
  constexpr float kNearPlane = 0.05f;

  struct CamPoint {
    float lateral;
    float depth;
    float height;
  };
}

namespace riz {

  Camera::Camera(float initFov) :
    pos(glm::vec2 {0, 0}), angle(0), fov(initFov) {}

  std::vector<WallQuad> Camera::worldToScreen(const std::vector<glm::vec3>& walls) const {
    std::vector<WallQuad> r;
    worldToScreen(walls, r);
    return r;
  }

  void Camera::worldToScreen(const std::vector<glm::vec3>& walls, std::vector<WallQuad>& vresult) const {
    vresult.clear();
    const std::size_t pairs = walls.size() / 2;
    vresult.reserve(pairs);

    const glm::vec2 f = forward();
    const glm::vec2 r = right();
    const float tanHalf = std::tan(fov / 2);

    auto toCamera = [&](const glm::vec3& p) {
      const glm::vec2 d = glm::vec2(p.x, p.y) - pos;
      return CamPoint { glm::dot(d, r), glm::dot(d, f), p[2] };
    };

    for (std::size_t i = 0; i < pairs; ++i) {
      CamPoint a = toCamera(walls[2 * i]);
      CamPoint b = toCamera(walls[2 * i + 1]);

      if (a.depth < kNearPlane && b.depth < kNearPlane)
        continue;

      if (a.depth < kNearPlane) {
        const float t = (kNearPlane - a.depth) / (b.depth - a.depth);
        a = CamPoint { a.lateral + (b.lateral - a.lateral) * t, kNearPlane,
                       a.height + (b.height - a.height) * t };
      } else if (b.depth < kNearPlane) {
        const float t = (kNearPlane - b.depth) / (a.depth - b.depth);
        b = CamPoint { b.lateral + (a.lateral - b.lateral) * t, kNearPlane,
                       b.height + (a.height - b.height) * t };
      }

      const float startX = a.lateral / (a.depth * tanHalf);
      const float startY = a.height / a.depth;
      const float endX = b.lateral / (b.depth * tanHalf);
      const float endY = b.height / b.depth;
      vresult.push_back(WallQuad { { startX, startY }, { startX, -startY }, { endX, endY }, { endX, -endY } });
    }
  }

  void Camera::setPosition(glm::vec2 position) {
    pos = position;
  }
  
  void Camera::setAngle(float newAngle) {
    angle = newAngle;
  }

  glm::vec2 Camera::getPosition() const {
    return pos;
  }

  float Camera::getAngle() const {
    return angle;
  }

  glm::vec2 Camera::forward() const {
    return glm::vec2(std::sin(angle), std::cos(angle));
  }

  glm::vec2 Camera::right() const {
    return glm::vec2(std::cos(angle), -std::sin(angle));
  }
}
