#include <shogle/render/framebuffer.hpp>

namespace ntf::render {

framebuffer::fbo_raii::fbo_raii(framebuffer* fbo) :
  _fbo(fbo) { _fbo->bind(); }

framebuffer::fbo_raii::~fbo_raii() { _fbo->unbind(); }

framebuffer::framebuffer(size_t w, size_t h) :
  _fbo(w, h) {}

} // namespace ntf::render
