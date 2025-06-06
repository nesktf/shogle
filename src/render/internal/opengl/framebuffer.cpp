#include "./context.hpp"

namespace ntf::render {

GLenum gl_state::fbo_attachment_cast(fbo_buffer att) noexcept {
  switch (att) {
    case fbo_buffer::none:     return 0;
    case fbo_buffer::depth16u: [[fallthrough]];
    case fbo_buffer::depth24u: [[fallthrough]];
    case fbo_buffer::depth32f: return GL_DEPTH_ATTACHMENT;
    case fbo_buffer::depth24u_stencil8u: [[fallthrough]];
    case fbo_buffer::depth32f_stencil8u: return GL_DEPTH_STENCIL_ATTACHMENT;
  }
  return 0;
}

ctx_fbo_status gl_state::create_framebuffer(glfbo_t& fbo, extent2d extent,
                                            fbo_buffer buffers, cspan<glfbo_att_t> attachments)
{
  const uint32 w = extent.x;
  const uint32 h = extent.y;
  NTF_ASSERT(attachments.size() <= MAX_FBO_ATTACHMENTS);
  NTF_ASSERT(attachments);
  NTF_ASSERT(w > 0 && h > 0);
  const GLenum sd_attachment = fbo_attachment_cast(buffers);
  const GLenum fbbind = GL_DRAW_FRAMEBUFFER;

  GLuint id;
  GL_CALL(glGenFramebuffers, 1, &id);
  GL_CALL(glBindFramebuffer, fbbind, id);
  _bound_fbos[GLFBO_BIND_WRITE] = id;

  GLuint rbo{0};
  if (sd_attachment) {
    GL_CALL(glGenRenderbuffers, 1, &rbo);
    GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbo);
    GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    GL_CALL(glFramebufferRenderbuffer, fbbind, sd_attachment, GL_RENDERBUFFER, rbo);
    GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, NULL_BINDING);
  }

  for (uint32 i = 0u; const auto& att : attachments) {
    NTF_ASSERT(att.tex);
    const auto& tex = *att.tex;
    texture_bind(tex.id, tex.type, _active_tex);
    switch (tex.type) {
      case GL_TEXTURE_1D: {
        NTF_ASSERT(w == tex.extent.x && h == 1);
        GL_CALL(glFramebufferTexture1D, fbbind, GL_COLOR_ATTACHMENT0+i,
                                        tex.type, tex.id, att.level);
        break;
      }
      case GL_TEXTURE_2D: {
        NTF_ASSERT(w == tex.extent.x && h == tex.extent.y);
        GL_CALL(glFramebufferTexture2D, fbbind, GL_COLOR_ATTACHMENT0+i,
                                        tex.type, tex.id, att.level);
        break;
      }
      default: {
        NTF_UNREACHABLE();
        break;
      }
    }
    ++i;
  }

  NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);

  fbo.id = id;
  fbo.sd_rbo = rbo;
  fbo.color_rbo = NULL_BINDING;
  fbo.extent.x = w;
  fbo.extent.y = h;

  return CTX_FBO_STATUS_OK;
}

void gl_state::destroy_framebuffer(glfbo_t& fbo) {
  NTF_ASSERT(fbo.id);
  if (_bound_fbos[GLFBO_BIND_WRITE] == fbo.id) {
    GL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, DEFAULT_FBO);
    _bound_fbos[GLFBO_BIND_WRITE] = DEFAULT_FBO;
  } else if (_bound_fbos[GLFBO_BIND_READ] == fbo.id) {
    GL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, DEFAULT_FBO);
    _bound_fbos[GLFBO_BIND_READ] = DEFAULT_FBO;
  }

  GLuint id = fbo.id;
  GL_CALL(glDeleteFramebuffers, 1, &id);
  if (fbo.sd_rbo) {
    GLuint rbo = fbo.sd_rbo;
    GL_CALL(glDeleteRenderbuffers, 1, &rbo);
  }
  if (fbo.color_rbo) {
    GLuint rbo = fbo.color_rbo;
    GL_CALL(glDeleteRenderbuffers, 1, &rbo);
  }
}

bool gl_state::framebuffer_bind(GLuint id, glfbo_binding binding) {
  GLenum fb;
  switch (binding) {
    case GLFBO_BIND_WRITE: {
      if (_bound_fbos[GLFBO_BIND_WRITE] == id) {
        return false;
      }
      fb = GL_DRAW_FRAMEBUFFER;
      _bound_fbos[GLFBO_BIND_WRITE] = id;
      break;
    }
    case GLFBO_BIND_READ: {
      if (_bound_fbos[GLFBO_BIND_READ] == id) {
        return false;
      }
      _bound_fbos[GLFBO_BIND_READ] = id;
      fb = GL_READ_FRAMEBUFFER;
      break;
    }
    case GLFBO_BIND_BOTH: {
      if (_bound_fbos[GLFBO_BIND_READ] == id && _bound_fbos[GLFBO_BIND_WRITE] == id) {
        return false;
      }
      _bound_fbos[GLFBO_BIND_READ] = id;
      _bound_fbos[GLFBO_BIND_WRITE] = id;
      fb = GL_FRAMEBUFFER;
      break;
    }
    default: {
      NTF_UNREACHABLE();
    }
  }
  GL_CALL(glBindFramebuffer, fb, id);
  return true;
}

void gl_state::framebuffer_prepare_state(GLuint fb, clear_flag flags,
                                         const uvec4& vp, const color4& col)
{
  framebuffer_bind(fb, GLFBO_BIND_WRITE);
  GL_CALL(glViewport, vp.x, vp.y, vp.z, vp.w);
  if (!+(flags & clear_flag::color)) {
    return;
  }
  GL_CALL(glClearColor, col.r, col.g, col.b, col.a);
  GL_CALL(glClear, clear_bit_cast(flags));
}

ctx_fbo_status gl_context::create_framebuffer(ctx_fbo& fbo, const ctx_fbo_desc& desc) {
  NTF_ASSERT(!desc.attachments.empty());
  auto atts = _alloc.arena_span<glfbo_att_t>(desc.attachments.size());
  for (size_t i = 0u; const auto& att : desc.ctx_attachments) {
    auto& tex = _textures.get(att.texture);
    atts[i].tex = &tex;
    atts[i].layer = att.layer;
    atts[i].level = att.level;
    ++i;
  }

  ctx_fbo handle = _framebuffers.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& framebuffer = _framebuffers.get(handle);
  const auto status = _state.create_framebuffer(framebuffer, desc.extent,
                                                desc.test_buffer, atts);
  if (status != CTX_FBO_STATUS_OK) {
    _framebuffers.push(handle);
    return status;
  }
  NTF_ASSERT(framebuffer.id);
  fbo = handle;
  return status;
}

ctx_fbo_status gl_context::destroy_framebuffer(ctx_fbo fbo) noexcept {
  if (!_framebuffers.validate(fbo)) {
    return CTX_FBO_STATUS_INVALID_HANDLE;
  }
  auto& framebuffer = _framebuffers.get(fbo);
  _state.destroy_framebuffer(framebuffer);
  _framebuffers.push(fbo);
  return CTX_FBO_STATUS_OK;
}

// auto gl_state::create_framebuffer(uint32 w, uint32 h, fbo_buffer buffers,
//                                   image_format format) -> framebuffer_t {
//   NTF_ASSERT(w > 0 && h > 0);
//   const GLenum sd_attachment = fbo_attachment_cast(buffers);
//   const GLenum fbbind = GL_DRAW_FRAMEBUFFER;
//
//   GLuint id;
//   GL_CALL(glGenFramebuffers, 1, &id);
//   GL_CALL(glBindFramebuffer, fbbind, id);
//   _bound_fbos[FBO_BIND_WRITE] = id;
//
//   GLuint rbos[2] = {0, 0};
//   if (sd_attachment) {
//     GL_CALL(glGenRenderbuffers, 2, rbos);
//     GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbos[0]);
//     GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
//     GL_CALL(glFramebufferRenderbuffer, fbbind, sd_attachment, GL_RENDERBUFFER, rbos[0]);
//   } else {
//     GL_CALL(glGenRenderbuffers, 1, &rbos[1]);
//   }
//   GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbos[1]);
//   GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, texture_format_cast(format), w, h);
//   GL_CALL(glFramebufferRenderbuffer, fbbind, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbos[1]);
//
//   GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, NULL_BINDING);
//
//   NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);
//
//   framebuffer_t fbo;
//   fbo.id = id;
//   fbo.sd_rbo = rbos[0];
//   fbo.color_rbo = rbos[1];
//   fbo.extent.x = w;
//   fbo.extent.y = h;
//   return fbo;
// }

} // namespace ntf::render
