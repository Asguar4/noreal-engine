#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <vector>

#include "./wallQuad.hpp"

namespace riz {
  class Camera {
    glm::vec2 pos;
    float angle;
    float fov;
  public:
    Camera(float initFov);

    std::vector<WallQuad> worldToScreen(const std::vector<glm::vec3>& walls) const;

    void worldToScreen(const std::vector<glm::vec3>& walls, std::vector<WallQuad>& out) const;

    void setPosition(glm::vec2 pos);
    void setAngle(float newAngle);

    glm::vec2 getPosition() const;
    float getAngle() const;
    glm::vec2 forward() const;
    glm::vec2 right() const;
  };
}
