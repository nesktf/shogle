#include <shogle/render/framebuffer.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#define DEFAULT_FRAMEBUFFER 0

namespace ntf::shogle {

framebuffer::framebuffer(size_t w, size_t h) :
  framebuffer(vec2sz{w, h}) {}

framebuffer::framebuffer(vec2sz sz) :
  _texture(nullptr, sz.w, sz.h, tex_format::rgb, tex_filter::nearest, tex_wrap::repeat),
  _size(sz) {
  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture.id(), 0);

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sz.w, sz.h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw ntf::error{"[shogle::framebuffer] Incomplete framebuffer"};
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  log::verbose("[shogle::framebuffer] Framebuffer created (id: {}, tex: {})", _fbo, _texture.id());
}

framebuffer::framebuffer(framebuffer&& f) noexcept :
  _texture(std::move(f._texture)),
  _fbo(std::move(f._fbo)), _rbo(std::move(f._rbo)), 
  _size(std::move(f._size)) { 
  f._fbo = 0; 
}

framebuffer& framebuffer::operator=(framebuffer&& f) noexcept {
  auto id = _fbo;

  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);
  
  _texture = std::move(f._texture);
  _fbo = std::move(f._fbo);
  _rbo = std::move(f._rbo);
  _size = std::move(f._size);

  f._fbo = 0;

  log::verbose("[shogle::framebuffer] Framebuffer overwritten (id: {}, tex: {})", id, _texture.id());

  return *this;
}

framebuffer::~framebuffer() {
  auto id = _fbo;
  if (!_fbo) return;
  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);
  log::verbose("[shogle::framebuffer] Framebuffer destroyed (id: {}, tex: {})", id, _texture.id());
}

framebuffer& framebuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  render_viewport(_size);
  return *this;
}

framebuffer& framebuffer::unbind(vec2sz viewport) {
  glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);
  render_viewport(viewport);
  return *this;
}

framebuffer::raii_bind framebuffer::scoped_bind(vec2sz viewport) { return raii_bind{*this, viewport}; }

framebuffer::raii_bind::raii_bind(framebuffer& fb, vec2sz viewport) :
  _fb(fb), _viewport(viewport) { fb.bind(); }

framebuffer::raii_bind::~raii_bind() { _fb.unbind(_viewport); }

} // namespace ntf::shogle
