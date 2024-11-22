#define SHOGLE_RENDER_OPENGL_FRAMEBUFFER_INL
#include "./framebuffer.hpp"
#undef SHOGLE_RENDER_OPENGL_FRAMEBUFFER_INL

namespace ntf {

template<framebuffer_func F>
gl_framebuffer& gl_framebuffer::bind(std::size_t vp_w, std::size_t vp_h, F&& func) & {
  _bind(vp_w, vp_h, std::forward<F>(func));
  return *this;
}

template<framebuffer_func F>
gl_framebuffer&& gl_framebuffer::bind(std::size_t vp_w, std::size_t vp_h, F&& func) && {
  _bind(vp_w, vp_h, std::forward<F>(func));
  return std::move(*this);
}

template<framebuffer_func F>
gl_framebuffer& gl_framebuffer::bind(ivec2 vp_sz, F&& func) & {
  _bind(vp_sz.x, vp_sz.y, std::forward<F>(func));
  return *this;
}

template<framebuffer_func F>
gl_framebuffer&& gl_framebuffer::bind(ivec2 vp_sz, F&& func) && {
  _bind(vp_sz.x, vp_sz.y, std::forward<F>(func));
  return std::move(*this);
}

template<framebuffer_func F>
void gl_framebuffer::_bind(std::size_t vp_w, std::size_t vp_h, F&& func) {
  NTF_ASSERT(valid());

  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  glViewport(0, 0, _dim.x, _dim.y);
  func();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, vp_w, vp_h);
}

} // namespace ntf
