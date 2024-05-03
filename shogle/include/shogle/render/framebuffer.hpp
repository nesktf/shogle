#pragma once

#include <shogle/render/sprite.hpp>

namespace ntf::render {

class framebuffer {
public:
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
  inline void bind(void) { gl::framebuffer_bind(_fbo.get()); }
  inline void unbind(void) { gl::framebuffer_bind(nullptr); }
  inline fbo_raii bind_scoped(void) { return fbo_raii{this}; }
  inline sprite* get_sprite(void) { return _fbo_sprite.get(); }

private:
  uptr<gl::framebuffer> _fbo;
  uptr<sprite> _fbo_sprite;
};

} // namespace ntf::render
