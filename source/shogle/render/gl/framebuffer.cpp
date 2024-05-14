#include <shogle/render/gl/framebuffer.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

#define DEFAULT_FRAMEBUFFER 0

namespace ntf::shogle::gl {

framebuffer::framebuffer(vec2sz sz) :
  _texture(sz, texture::type::tex2d, texture::format::rgb),
  _size(sz) {
  _texture.set_filter(texture::filter::nearest);

  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _texture.type(), _texture.id(), 0);

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sz.w, sz.h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw ntf::error{"[gl::framebuffer] Incomplete framebuffer"};
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  Log::verbose("[gl::framebuffer] Framebuffer created (id: {}, tex-id: {})", _fbo, _texture.id());
}

framebuffer::framebuffer(framebuffer&& f) noexcept :
  _texture(std::move(f._texture)),
  _fbo(f._fbo), _rbo(f._rbo), _size(f._size) { f._fbo = 0; }

framebuffer& framebuffer::operator=(framebuffer&& f) noexcept {
  Log::verbose("[gl::framebuffer] Framebuffer overwritten (id: {}, tex-id: {})", _fbo, _texture.id());
  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);
  
  _texture = std::move(f._texture);
  _fbo = f._fbo;
  _rbo = f._rbo;
  _size = f._size;

  f._fbo = 0;

  return *this;
}

framebuffer::~framebuffer() {
  if (!_fbo) return;
  Log::verbose("[gl::framebuffer] Framebuffer destroyed (id: {}, tex-id: {})", _fbo, _texture.id());
  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);
}

framebuffer& framebuffer::bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  return *this;
}

framebuffer& framebuffer::unbind() {
  glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FRAMEBUFFER);
  return *this;
}

framebuffer::raii_bind framebuffer::scoped_bind() { return raii_bind{*this}; }

framebuffer::raii_bind::raii_bind(framebuffer& fb) :
  _fb(fb) { fb.bind(); }

framebuffer::raii_bind::~raii_bind() { _fb.unbind(); }

} // namespace ntf::shogle::gl
