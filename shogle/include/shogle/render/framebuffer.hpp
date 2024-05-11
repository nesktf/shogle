#pragma once

#include <shogle/render/sprite.hpp>

namespace ntf::render {

struct framebuffer {
public:
  struct fbo_raii {
    fbo_raii(framebuffer* fbo, size_t win_w, size_t win_h);
    ~fbo_raii();
    framebuffer* _fbo;
    size_t w, h;
  };

public:
  framebuffer(size_t w, size_t h);

public:
  inline void bind(void) { 
    gl::set_viewport(width(), height());
    gl::framebuffer_bind(&_fbo); 
  }
  inline void unbind(size_t def_w, size_t def_h) { 
    gl::framebuffer_bind(nullptr); 
    gl::set_viewport(def_w, def_h);
  }
  inline fbo_raii bind_scoped(size_t win_w, size_t win_h) { return fbo_raii{this, win_w, win_h}; }

  inline size_t width(void) { return _fbo.tex.width; }
  inline size_t height(void) { return _fbo.tex.height; }

  inline sprite* get_sprite(void) { return &_fbo_sprite; }

public:
  gl::framebuffer _fbo;
  sprite _fbo_sprite;

public:
  ~framebuffer();
  framebuffer(framebuffer&&) = default;
  framebuffer(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) = delete;
};

} // namespace ntf::render
