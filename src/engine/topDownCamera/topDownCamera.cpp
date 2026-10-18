#include "../topDownCamera.hpp"

#include <cmath>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

namespace riz {
  TopDownCamera::TopDownCamera(float initViewHeight)
    : pos(0.f, 0.f), angle(0.f), zoom(1.f), viewHeight(initViewHeight), viewport(1, 1) {
    if (!std::isfinite(initViewHeight) || initViewHeight <= 0.f) {
      throw std::invalid_argument("TopDownCamera: view height must be finite and positive");
    }
  }

  void TopDownCamera::setPosition(glm::vec2 center) {
    pos = center;
  }

  void TopDownCamera::setAngle(float newAngle) {
    angle = newAngle;
  }

  void TopDownCamera::setZoom(float newZoom) {
    if (!std::isfinite(newZoom) || newZoom <= 0.f) {
      return;
    }
    zoom = newZoom;
  }

  void TopDownCamera::setViewportSize(glm::ivec2 size) {
    if (size.x <= 0 || size.y <= 0) {
      return;
    }
    viewport = size;
  }

  glm::vec2 TopDownCamera::getPosition() const {
    return pos;
  }

  float TopDownCamera::getAngle() const {
    return angle;
  }

  float TopDownCamera::getZoom() const {
    return zoom;
  }

  float TopDownCamera::getViewHeight() const {
    return viewHeight;
  }

  glm::ivec2 TopDownCamera::getViewportSize() const {
    return viewport;
  }

  float TopDownCamera::aspectRatio() const {
    return static_cast<float>(viewport.x) / static_cast<float>(viewport.y);
  }

  glm::vec2 TopDownCamera::visibleHalfExtent() const {
    const float hh = viewHeight / (2.f * zoom);
    return glm::vec2(hh * aspectRatio(), hh);
  }

  glm::vec2 TopDownCamera::worldToScreen(glm::vec2 world) const {
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const glm::vec2 d = world - pos;
    const glm::vec2 he = visibleHalfExtent();
    const float lx = d.x * c - d.y * s;
    const float ly = d.x * s + d.y * c;
    return glm::vec2(lx / he.x, ly / he.y);
  }

  glm::vec2 TopDownCamera::screenToWorld(glm::vec2 screen) const {
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const glm::vec2 he = visibleHalfExtent();
    const glm::vec2 l = screen * he;
    return pos + glm::vec2(c * l.x + s * l.y, -s * l.x + c * l.y);
  }

  std::vector<glm::vec2> TopDownCamera::worldToScreen(const std::vector<glm::vec2>& points) const {
    std::vector<glm::vec2> out;
    worldToScreen(points, out);
    return out;
  }

  void TopDownCamera::worldToScreen(const std::vector<glm::vec2>& points, std::vector<glm::vec2>& out) const {
    out.clear();
    out.reserve(points.size());
    for (const glm::vec2& p : points) {
      out.push_back(worldToScreen(p));
    }
  }

  glm::vec2 TopDownCamera::worldToPixel(glm::vec2 world) const {
    const glm::vec2 s = worldToScreen(world);
    return glm::vec2((s.x + 1.f) / 2.f * static_cast<float>(viewport.x),
                     (1.f - s.y) / 2.f * static_cast<float>(viewport.y));
  }

  glm::vec2 TopDownCamera::pixelToWorld(glm::vec2 pixel) const {
    const glm::vec2 s(pixel.x / static_cast<float>(viewport.x) * 2.f - 1.f,
                      1.f - pixel.y / static_cast<float>(viewport.y) * 2.f);
    return screenToWorld(s);
  }

  glm::mat4 TopDownCamera::viewProjection() const {
    const glm::vec2 he = visibleHalfExtent();
    const glm::mat4 proj = glm::ortho(-he.x, he.x, -he.y, he.y, -1.f, 1.f);
    const glm::mat4 view = glm::rotate(glm::mat4(1.f), angle, glm::vec3(0.f, 0.f, 1.f)) *
                           glm::translate(glm::mat4(1.f), glm::vec3(-pos, 0.f));
    return proj * view;
  }
}
