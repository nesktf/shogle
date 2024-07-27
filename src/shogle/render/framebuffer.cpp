#include <shogle/render/framebuffer.hpp>

#include <shogle/core/log.hpp>
#include <shogle/core/error.hpp>

namespace ntf::shogle {

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

void framebuffer::unload_framebuffer() {
  log::verbose("[shogle::framebuffer] Framebuffer destroyed (id: {}, tex: {})", _fbo, _texture.id());
  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);
}

void render_bind_sampler(const framebuffer& fb, size_t sampler) {
  glActiveTexture(GL_TEXTURE0+sampler);
  glBindTexture(GL_TEXTURE_2D, fb._texture._id);
}

} // namespace ntf::shogle
