#define SHOGLE_RENDER_FRAMEBUFFER_INL
#include <shogle/render/gl/framebuffer.hpp>
#undef SHOGLE_RENDER_FRAMEBUFFER_INL

namespace ntf {

inline gl::framebuffer::framebuffer(size_t w, size_t h) : 
  _texture(nullptr, ivec2{w, h}, tex_format::rgb), _dim(w, h) {
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
    ntf::log::error("[SHOGLE][ntf::gl::framebuffer] Failed to create, incomplete (id: {})", _fbo);
    _fbo = 0;
    _rbo = 0;
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  SHOGLE_INTERNAL_LOG_FMT(verbose,
                          "[SHOGLE][ntf::gl::framebuffer] Created (id: {}, tex: {})", _fbo, _texture.id());
}

inline gl::framebuffer::~framebuffer() noexcept {
#ifdef SHOGLE_GL_RAII_UNLOAD
  unload();
#endif
}

inline gl::framebuffer::framebuffer(framebuffer&& f) noexcept :
  _fbo(std::move(f._fbo)), _rbo(std::move(f._rbo)), 
  _texture(std::move(f._texture)), _dim(std::move(f._dim)) {
  f._fbo = 0;
  f._rbo = 0;
}

inline auto gl::framebuffer::operator=(framebuffer&& f) noexcept -> framebuffer& {
  unload();
  
  _texture = std::move(f._texture);
  _fbo = std::move(f._fbo);
  _rbo = std::move(f._rbo);
  _dim = std::move(f._dim);

  f._fbo = 0;
  f._rbo = 0;

  return *this;
}

inline void gl::framebuffer::unload() {
  if (_fbo && _rbo) {
    SHOGLE_INTERNAL_LOG_FMT(verbose,
                            "[SHOGLE][ntf::gl::framebuffer] Destroyed (id: {}, tex: {})", _fbo, _texture.id());
    glDeleteFramebuffers(1, &_fbo);
    glDeleteBuffers(1, &_rbo);
    _fbo = 0;
    _rbo = 0;
    _texture.unload();
  }
}

template<typename Renderer>
void gl::framebuffer::bind(ivec2 vp_sz, Renderer&& renderer) {
  assert(valid() && "Invalid Framebuffer");
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  gl::set_viewport(_dim);
  renderer();
  glBindFramebuffer(GL_FRAMEBUFFER, gl::DEFAULT_FRAMEBUFFER);
  gl::set_viewport(vp_sz);
}

template<typename Renderer>
void gl::framebuffer::bind(size_t viewport_w, size_t viewport_h, Renderer&& renderer) {
  bind(ivec2{static_cast<int>(viewport_w), static_cast<int>(viewport_h)}, std::forward<Renderer>(renderer));
}

} // namespace ntf
