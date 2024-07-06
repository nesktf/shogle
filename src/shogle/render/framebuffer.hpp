#pragma once

#include <shogle/render/texture.hpp>

#define SHOGLE_DEFAULT_FRAMEBUFFER 0

namespace ntf::shogle {

class framebuffer {
public:
  framebuffer(size_t w, size_t h);

public:
  template<typename F>
  void bind(size_t viewport_w, size_t viewport_h, F&& fun) {
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    render_viewport(_w, _h);
    fun();
    glBindFramebuffer(GL_FRAMEBUFFER, SHOGLE_DEFAULT_FRAMEBUFFER);
    render_viewport(viewport_w, viewport_h);
  }

  texture2d& tex() { return _texture; }

public:
  GLuint id() const { return _fbo; }
  vec2sz size() const { return vec2sz{_w, _h}; }

public:
  ~framebuffer();
  framebuffer(framebuffer&&) noexcept;
  framebuffer(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) = delete;

private:
  texture2d _texture;
  GLuint _fbo, _rbo;
  size_t _w, _h;
};

} // namespace ntf::shogle
