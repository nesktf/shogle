#pragma once

#include <shogle/render/texture.hpp>

#define SHOGLE_DEFAULT_FRAMEBUFFER 0

namespace ntf::shogle {

class framebuffer {
public:
  framebuffer() = default;
  framebuffer(GLuint fbo, GLuint rbo, GLuint texture, size_t w, size_t h);
  framebuffer(size_t w, size_t h);

public:
  template<typename F>
  void bind(size_t viewport_w, size_t viewport_h, F&& fun) {
    assert(valid() && "Invalid Framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    render_viewport(_dim.x, _dim.y);
    fun();
    glBindFramebuffer(GL_FRAMEBUFFER, SHOGLE_DEFAULT_FRAMEBUFFER);
    render_viewport(viewport_w, viewport_h);
  }

  const texture2d& tex() const { return _texture; }

public:
  GLuint& id() { return _fbo; } // Not const
  ivec2 size() const { return _dim; }
  bool valid() const { return _fbo != 0 && _fbo != 0 && _texture.valid(); }

public:
  ~framebuffer();
  framebuffer(framebuffer&&) noexcept;
  framebuffer(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) = delete;

private:
  void unload_framebuffer();

private:
  GLuint _fbo{}, _rbo{};
  texture2d _texture{};
  ivec2 _dim{};

private:
  friend void render_bind_sampler(const framebuffer& fb, size_t sampler);
};

void render_bind_sampler(const framebuffer& fb, size_t sampler);

} // namespace ntf::shogle
