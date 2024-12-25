#include "./framebuffer.hpp"
#include "./context.hpp"

namespace ntf {

void gl_framebuffer::load(uint32 x, uint32 y, uint32 w, uint32 h,
                          r_texture_sampler sampler, r_texture_address address) {
  NTF_ASSERT(!_fbo);

  auto tex = _ctx.make_texture(r_texture_descriptor{
    .texels = nullptr,
    .count = 1,
    .mipmap_level = 1,
    .extent = uvec3{w, h, 0},
    .type = r_texture_type::texture2d,
    .format = r_texture_format::rgb,
    .sampler = sampler,
    .addressing = address,
  });
  if (!tex) {
    return;
  }

  GLuint fbo, rbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (**tex)._id, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    SHOGLE_LOG(error, "[ntf::gl_framebuffer] Failed to create framebuffer (id: {})", fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  _tex = std::move(*tex);
  _fbo = fbo;
  _rbo = rbo;
  _viewport.x = x;
  _viewport.y = y;
  _viewport.z = w;
  _viewport.w = h;
}

void gl_framebuffer::unload() {
  NTF_ASSERT(_fbo);

  glDeleteFramebuffers(1, &_fbo);
  glDeleteBuffers(1, &_rbo);

  _tex.reset();
  _fbo = 0;
  _rbo = 0;
  _viewport = uvec4{0, 0, 0, 0};
  _clear = r_clear::none;
  _clear_color = color4{.3f, .3f, .3f, 1.f};
}

void gl_framebuffer::viewport(uint32 x, uint32 y, uint32 w, uint32 h) {
  const bool rebuild = (_viewport.z != w || _viewport.w != h);

  _viewport.x = x;
  _viewport.y = y;
  _viewport.z = w;
  _viewport.w = h;

  if (!rebuild) {
    return;
  }

  NTF_ASSERT(_fbo);

  auto tex = _ctx.make_texture(r_texture_descriptor{
    .texels = nullptr,
    .count = 1,
    .mipmap_level = 1,
    .extent = uvec3{w, h, 0},
    .type = r_texture_type::texture2d,
    .format = r_texture_format::rgb,
    .sampler = _tex->sampler(),
    .addressing = _tex->addressing(),
  });
  if (!tex) {
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

  // TODO: Copy the old texture pixels to the new one?
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (**tex)._id, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  _tex = std::move(*tex);

}

void gl_framebuffer::viewport(uint32 w, uint32 h) {
  viewport(_viewport.x, _viewport.y, w, h);
}

void gl_framebuffer::viewport(uvec2 pos, uvec2 size) {
  viewport(pos.x, pos.y, size.x, size.y);
}

void gl_framebuffer::viewport(uvec2 size) {
  viewport(size.x, size.y);
}

void gl_framebuffer::clear_color(float32 r, float32 g, float32 b, float32 a) {
  _clear_color.r = r;
  _clear_color.g = g;
  _clear_color.b = b;
  _clear_color.a = a;
}

void gl_framebuffer::clear_color(float32 r, float32 g, float32 b) {
  clear_color(r, g, b, _clear_color.a);
}

void gl_framebuffer::clear_color(color4 color) {
  clear_color(color.r, color.g, color.b, color.a);
}

void gl_framebuffer::clear_color(color3 color) {
  clear_color(color.r, color.g, color.b);
}

void gl_framebuffer::clear_flags(r_clear clear) {
  _clear = clear;
}

} // namespace ntf
