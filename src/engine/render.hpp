#pragma once

#include <string>

#include <glm/vec2.hpp>

#include "../containers/vector.hpp"
#include "./render/iRender.hpp"

namespace riz {
  // Precondition for every member: an OpenGL 3.3 core context is current on the calling thread
  // (in practice the one created by riz::Window), and that context outlives this Renderer.
  // Declare the riz::Window before the Renderer so the GL objects are released before the context goes away.
  class Renderer {
  public:
    // glewExperimental = GL_TRUE; glewInit (throws "GLEW initialization failed: ..."); drain glGetError;
    // glViewport(0, 0, fb.x, fb.y); batch.init(...). Throws std::runtime_error. No GL object stays alive after a throw.
    Renderer(glm::ivec2 framebufferSize, const std::string& pathVert, const std::string& pathFrag);
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    void begin();
    void draw(const Vector<glm::vec2>& quadVertices);   // draws immediately, stores nothing

  private:
    RenderBatch batch;
  };
}
