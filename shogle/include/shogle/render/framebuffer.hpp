#pragma once

#include <shogle/render/backends/gl.hpp>

namespace ntf::render {

class framebuffer {
public:
  using renderer = gl;

  class fbo_raii {
    public:
      fbo_raii(framebuffer* fbo);
      ~fbo_raii();
    private:
      framebuffer* _fbo;
  };

public:
  framebuffer(size_t w, size_t h);

public:
  inline void bind(void) { renderer::framebuffer_bind(&_fbo); }
  inline void unbind(void) { renderer::framebuffer_bind(nullptr); }
  inline fbo_raii bind_scoped(void) { return fbo_raii{this}; }

private:
  renderer::framebuffer _fbo;
};

} // namespace ntf::render
