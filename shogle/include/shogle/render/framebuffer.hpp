#pragma once

#include <shogle/render/sprite.hpp>

namespace ntf::render {

struct framebuffer {
public:
  struct fbo_raii {
    fbo_raii(framebuffer* fbo);
    ~fbo_raii();
    framebuffer* _fbo;
  };

public:
  framebuffer(size_t w, size_t h);

public:
  inline void bind(void) { gl::framebuffer_bind(&_fbo); }
  inline void unbind(void) { gl::framebuffer_bind(nullptr); }
  inline fbo_raii bind_scoped(void) { return fbo_raii{this}; }

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
