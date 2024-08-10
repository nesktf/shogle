#pragma once

#include <shogle/render/texture.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#define SHOGLE_DEFAULT_FRAMEBUFFER 0

namespace ntf {

class framebuffer {
public:
  framebuffer() = default;
  inline framebuffer(GLuint fbo, GLuint rbo, GLuint texture, size_t w, size_t h);
  inline framebuffer(size_t w, size_t h);

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
  inline ~framebuffer();
  inline framebuffer(framebuffer&&) noexcept;
  framebuffer(const framebuffer&) = delete;
  inline framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) = delete;

private:
  inline void unload_framebuffer();

private:
  GLuint _fbo{}, _rbo{};
  texture2d _texture{};
  ivec2 _dim{};

private:
  friend void render_bind_sampler(const framebuffer& fb, size_t sampler);
};

framebuffer::framebuffer(GLuint fbo, GLuint rbo, GLuint texture, size_t w, size_t h) : 
  _fbo(fbo), _rbo(rbo), _texture(texture, w, h) {}

framebuffer::framebuffer(size_t w, size_t h) : _texture(nullptr, w, h, tex_format::rgb), _dim(w, h) {
  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture.id(), 0);

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw ntf::error{"[shogle::framebuffer] Incomplete framebuffer"};
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  log::verbose("[shogle::framebuffer] Framebuffer created (id: {}, tex: {})", _fbo, _texture.id());
}

framebuffer::framebuffer(framebuffer&& f) noexcept :
  _fbo(std::move(f._fbo)), _rbo(std::move(f._rbo)), 
  _texture(std::move(f._texture)), _dim(std::move(f._dim)) {
  f._fbo = 0;
  f._rbo = 0;
}

framebuffer& framebuffer::operator=(framebuffer&& f) noexcept {
  if (_fbo && _rbo) {
    unload_framebuffer();
  }
  
  _texture = std::move(f._texture);
  _fbo = std::move(f._fbo);
  _rbo = std::move(f._rbo);
  _dim = std::move(f._dim);

  f._fbo = 0;
  f._rbo = 0;

  return *this;
}

framebuffer::~framebuffer() {
  if (_fbo && _rbo) {
    unload_framebuffer();
  }
}


inline void framebuffer::unload_framebuffer() {
  log::verbose("[shogle::framebuffer] Framebuffer destroyed (id: {}, tex: {})", _fbo, _texture.id());
  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);
}

inline void render_bind_sampler(const framebuffer& fb, size_t sampler) {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(GL_TEXTURE_2D, fb._texture._id);
}

} // namespace ntf
