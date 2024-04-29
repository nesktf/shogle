#pragma once

#include <shogle/render/backends/gl.hpp>
#include <shogle/render/sprite.hpp>

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
  inline void bind(void) { renderer::framebuffer_bind(_fbo.get()); }
  inline void unbind(void) { renderer::framebuffer_bind(nullptr); }
  inline fbo_raii bind_scoped(void) { return fbo_raii{this}; }
  inline sprite* get_sprite(void) { return _fbo_sprite.get(); }

private:
  uptr<renderer::framebuffer> _fbo;
  uptr<sprite> _fbo_sprite;
};

} // namespace ntf::render
