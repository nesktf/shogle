#include <shogle/render/framebuffer.hpp>

namespace ntf::render {

framebuffer::fbo_raii::fbo_raii(framebuffer* fbo) :
  _fbo(fbo) { _fbo->bind(); }

framebuffer::fbo_raii::~fbo_raii() { _fbo->unbind(); }

framebuffer::framebuffer(size_t w, size_t h) :
  _fbo(w, h),
  _fbo_sprite(_fbo.tex, w, h) {}

framebuffer& framebuffer::operator=(framebuffer&& f) noexcept {
  gl::destroy_framebuffer(_fbo);

  _fbo = std::move(f._fbo);
  _fbo_sprite = std::move(f._fbo_sprite);

  f._fbo.fbo = 0; // avoid destroying moved gl handle

  return *this;
}

framebuffer::~framebuffer() {
  gl::destroy_framebuffer(_fbo);
}

} // namespace ntf::render
