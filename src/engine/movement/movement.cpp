#include <cmath>

#include <glm/vec2.hpp>

#include "../movement.hpp"

namespace riz {
  glm::vec2 moveDirection(float forwardAxis, float rightAxis, glm::vec2 forward, glm::vec2 right) noexcept {
    const float lengthSquared = forwardAxis * forwardAxis + rightAxis * rightAxis;
    if (lengthSquared > 1.f) {
      const float inv = 1.f / std::sqrt(lengthSquared);
      forwardAxis *= inv;
      rightAxis *= inv;
    }
    return forwardAxis * forward + rightAxis * right;
  }
}
