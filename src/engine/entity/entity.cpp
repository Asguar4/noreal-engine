#define GLM_ENABLE_EXPERIMENTAL

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../entity.hpp"
#include "../array.hpp"

namespace riz {
/*
  Body::Body(float initSpeed, float initSens) :
    pos(glm::vec2 {0, 0}), angle(0), speed(initSpeed), sens(initSens) {}

  const glm::vec2& Body::getPosition() const {
    return pos;
  }

  float glm::vec2 Body::getAngle() {
    return angle;
  }
*/
  Camera::Camera(float initFov) :
    pos(glm::vec2 {0, 0}), angle(0), fov(initFov) {}

  Array<glm::vec2> Camera::worldToScreen(Array<glm::vec3> vertices) const {
    Array<glm::vec2> vresult(vertices.size() * 2);

    size_t lenR = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
      glm::vec2 dpos {vertices[i][0] - pos[0], vertices[i][1] - pos[1]};
      dpos = glm::rotate(dpos, angle);
      
      float x = dpos[0] / (dpos[1] * tan(fov / 2));
      float y = vertices[i][2] / dpos[1];

      vresult[lenR++] = glm::vec2{x,  y};
      vresult[lenR++] = glm::vec2{x, -y};
    }

    return vresult;
  }

  void Camera::setPosition(glm::vec2 position) {
    pos = position;
  }
  
  void Camera::setAngle(float newAngle) {
    angle = newAngle;
  }
}
