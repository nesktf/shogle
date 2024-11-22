#include "./framebuffer.hpp"

namespace ntf {

gl_framebuffer::gl_framebuffer(std::size_t w, std::size_t h, gl_tex_params params) {
  _load(w, h, params);
}

gl_framebuffer::gl_framebuffer(ivec2 sz, gl_tex_params params) :
  gl_framebuffer(sz.x, sz.y, params) {}

gl_framebuffer& gl_framebuffer::load(std::size_t w, std::size_t h, gl_tex_params params) & {
  _load(w, h, params);
  return *this;
}

gl_framebuffer&& gl_framebuffer::load(std::size_t w, std::size_t h, gl_tex_params params) && {
  _load(w, h, params);
  return std::move(*this);
}

gl_framebuffer& gl_framebuffer::load(ivec2 sz, gl_tex_params params) & {
  _load(sz.x, sz.y, params);
  return *this;
}

gl_framebuffer&& gl_framebuffer::load(ivec2 sz, gl_tex_params params) && {
  _load(sz.x, sz.y, params);
  return std::move(*this);
}

void gl_framebuffer::unload() {
  if (!valid()) {
    return;
  }

  SHOGLE_LOG(verbose, "[ntf::gl_framebuffer] Framebuffer destroyed (id: {}, tex: {})",
             _fbo, _texture.id());
  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);

  _reset();
}


void gl_framebuffer::_load(std::size_t w, std::size_t h, gl_tex_params params) {
  auto tex = texture_type{}.load(nullptr, ivec2{w, h}, tex_format::rgb, params);
  if (!_texture.valid()) {
    SHOGLE_LOG(error, "[ntf::framebuffer] Failed to create texture");
    return;
  }

  GLuint fbo{}, rbo{};
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.id(), 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    SHOGLE_LOG(error, "[ntf::gl_framebuffer] Failed to create framebuffer (incomplete) (id: {})",
               fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    tex.unload();
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (_fbo) {
    SHOGLE_LOG(warning, "[ntf::gl_framebuffer] Framebuffer overwritten ({} -> {})", _fbo, fbo);
    glDeleteFramebuffers(1, &_fbo);
    glDeleteBuffers(1, &_rbo);
  }

  SHOGLE_LOG(verbose, "[ntf::gl_framebuffer] Framebuffer created (id: {}, tex: {})",
             fbo, tex.id());

  _fbo = fbo;
  _rbo = rbo;
  _texture = std::move(tex);
  _dim = ivec2{w, h};
}

void gl_framebuffer::_reset() {
  _fbo = 0;
  _rbo = 0;
  _texture.unload();
  _dim = ivec2{0,0};
}


gl_framebuffer::~gl_framebuffer() noexcept { unload(); }

gl_framebuffer::gl_framebuffer(gl_framebuffer&& f) noexcept :
  _fbo(std::move(f._fbo)), _rbo(std::move(f._rbo)), 
  _texture(std::move(f._texture)), _dim(std::move(f._dim)) { f._reset(); }

auto gl_framebuffer::operator=(gl_framebuffer&& f) noexcept -> gl_framebuffer& {
  unload();
  
  _texture = std::move(f._texture);
  _fbo = std::move(f._fbo);
  _rbo = std::move(f._rbo);
  _dim = std::move(f._dim);

  f._reset();

  return *this;
}

} // namespace ntf
