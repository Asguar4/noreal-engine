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

  std::vector<glm::vec2> Camera::worldToScreen(const std::vector<glm::vec3>& walls) const {
    std::vector<glm::vec2> r;
    worldToScreen(walls, r);
    return r;
  }

  void Camera::worldToScreen(const std::vector<glm::vec3>& walls, std::vector<glm::vec2>& vresult) const {
    vresult.clear();
    const std::size_t pairs = walls.size() / 2;
    vresult.reserve(pairs * 4);

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

      for (const CamPoint& pt : { a, b }) {
        const float x = pt.lateral / (pt.depth * tanHalf);
        const float y = pt.height / pt.depth;
        vresult.emplace_back(x, y);
        vresult.emplace_back(x, -y);
      }
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
