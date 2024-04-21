#include "scene/framebuffer.hpp"

#include "core/log.hpp"

namespace ntf {

Framebuffer::Framebuffer(size_t width, size_t height, GLint filter, GLenum dim) :
  Texture(width, height, filter, dim) {
  inverted = true;

  glGenFramebuffers(1, &_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dim, _tex, 0);

  glGenRenderbuffers(1, &_rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _w, _h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    Log::error("[Framebuffer] Incomplete framebuffer!");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  cam2D.set_viewport({static_cast<float>(width), static_cast<float>(height)});
  cam3D.set_viewport({static_cast<float>(width), static_cast<float>(height)});
}

} // namespace ntf
