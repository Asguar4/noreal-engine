#pragma once

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace riz {
  class TopDownCamera {
    glm::vec2 pos;
    float angle;
    float zoom;
    float viewHeight;
    glm::ivec2 viewport;
  public:
    explicit TopDownCamera(float initViewHeight);

    void setPosition(glm::vec2 center);
    void setAngle(float newAngle);
    void setZoom(float newZoom);
    void setViewportSize(glm::ivec2 size);

    glm::vec2 getPosition() const;
    float getAngle() const;
    float getZoom() const;
    float getViewHeight() const;
    glm::ivec2 getViewportSize() const;
    float aspectRatio() const;
    glm::vec2 visibleHalfExtent() const;

    glm::vec2 worldToScreen(glm::vec2 world) const;
    glm::vec2 screenToWorld(glm::vec2 screen) const;
    std::vector<glm::vec2> worldToScreen(const std::vector<glm::vec2>& points) const;
    void worldToScreen(const std::vector<glm::vec2>& points, std::vector<glm::vec2>& out) const;

    glm::vec2 worldToPixel(glm::vec2 world) const;
    glm::vec2 pixelToWorld(glm::vec2 pixel) const;

    glm::mat4 viewProjection() const;
  };
}
