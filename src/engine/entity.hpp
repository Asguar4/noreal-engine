#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "../containers/array.hpp"

namespace riz {
  struct Wall {
    glm::vec2 pos1;
    glm::vec2 pos2;
    float height;
  };

  class Body {
    glm::vec2 pos;
    float angle;
    float speed;
    float sens;

    glm::vec2 move();
    glm::vec2 rotate();
  public:
    Body(float initSpeed, float initSens);

    void update();

    const glm::vec2& getPosition() const;
    float getAngle() const;
  };
  
  class Camera {
    glm::vec2 pos;
    float angle;
    float fov;
  public:
    Camera(float initFov);

    // initFov is in radians. Consecutive pairs of walls points form one wall.
    Vector<glm::vec2> worldToScreen(const Vector<glm::vec3>& walls) const;

    void setPosition(glm::vec2 pos);
    void setAngle(float newAngle);
  };
}
