#include "./framebuffer.hpp"

namespace ntf {

void gl_framebuffer::load(ivec2 sz) { load(sz.x, sz.y); }

void gl_framebuffer::load(std::size_t w, std::size_t h) {
  NTF_ASSERT(_fbo == 0 && _rbo == 0 && !_texture.valid(), "gl_framebuffer already initialized");

  _texture.load(nullptr, ivec2{w, h}, tex_format::rgb);
  if (!_texture.valid()) {
    return;
  }

  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture.id(), 0);

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &_fbo);
    glDeleteRenderbuffers(1, &_rbo);
    _texture.unload();
    SHOGLE_LOG(error, "[ntf::gl_framebuffer] Failed to create, incomplete (id: {})", _fbo);
    _fbo = 0;
    _rbo = 0;
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  SHOGLE_LOG(verbose, "[ntf::gl_framebuffer] Created (id: {}, tex: {})",_fbo, _texture.id());
}

void gl_framebuffer::unload() {
  if (_fbo && _rbo) {
    SHOGLE_LOG(verbose, "[ntf::gl_framebuffer] Destroyed (id: {}, tex: {})", _fbo, _texture.id());
    glDeleteFramebuffers(1, &_fbo);
    glDeleteBuffers(1, &_rbo);
    _fbo = 0;
    _rbo = 0;
    _texture.unload();
  }
}

gl_framebuffer::~gl_framebuffer() noexcept { unload(); }

gl_framebuffer::gl_framebuffer(gl_framebuffer&& f) noexcept :
  _fbo(std::move(f._fbo)), _rbo(std::move(f._rbo)), 
  _texture(std::move(f._texture)), _dim(std::move(f._dim)) {
  f._fbo = 0;
  f._rbo = 0;
}

auto gl_framebuffer::operator=(gl_framebuffer&& f) noexcept -> gl_framebuffer& {
  unload();
  
  _texture = std::move(f._texture);
  _fbo = std::move(f._fbo);
  _rbo = std::move(f._rbo);
  _dim = std::move(f._dim);

  f._fbo = 0;
  f._rbo = 0;

  return *this;
}

} // namespace ntf
