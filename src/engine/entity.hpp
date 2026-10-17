#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../containers/vector.hpp"

namespace riz {
  class Camera {
    glm::vec2 pos;
    float angle;
    float fov;
  public:
    Camera(float initFov);

    // initFov is in radians. Consecutive pairs of walls points form one wall.
    Vector<glm::vec2> worldToScreen(const Vector<glm::vec3>& walls) const;

    // Same output as the returning overload. Clears `out` with out.resize(0), which keeps its capacity, then appends.
    void worldToScreen(const Vector<glm::vec3>& walls, Vector<glm::vec2>& out) const;

    void setPosition(glm::vec2 pos);
    void setAngle(float newAngle);
  };
}
