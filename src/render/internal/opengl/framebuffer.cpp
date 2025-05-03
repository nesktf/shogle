#include "./state.hpp"

namespace ntf {

GLenum gl_state::fbo_attachment_cast(r_test_buffer att) noexcept {
  switch (att) {
    case r_test_buffer::no_buffer: return 0;
    case r_test_buffer::depth16u: [[fallthrough]];
    case r_test_buffer::depth24u: [[fallthrough]];
    case r_test_buffer::depth32f: return GL_DEPTH_ATTACHMENT;
    case r_test_buffer::depth24u_stencil8u: [[fallthrough]];
    case r_test_buffer::depth32f_stencil8u: return GL_DEPTH_STENCIL_ATTACHMENT;
  }
  return 0;
}

auto gl_state::create_framebuffer(uint32 w, uint32 h, r_test_buffer buffers,
                                  r_texture_format format) -> framebuffer_t {
  NTF_ASSERT(w > 0 && h > 0);
  const GLenum sd_attachment = fbo_attachment_cast(buffers);
  const GLenum fbbind = GL_DRAW_FRAMEBUFFER;

  GLuint id;
  GL_CALL(glGenFramebuffers, 1, &id);
  GL_CALL(glBindFramebuffer, fbbind, id);
  _bound_fbos[FBO_BIND_WRITE] = id;

  GLuint rbos[2] = {0, 0};
  if (sd_attachment) {
    GL_CALL(glGenRenderbuffers, 2, rbos);
    GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbos[0]);
    GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    GL_CALL(glFramebufferRenderbuffer, fbbind, sd_attachment, GL_RENDERBUFFER, rbos[0]);
  } else {
    GL_CALL(glGenRenderbuffers, 1, &rbos[1]);
  }
  GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbos[1]);
  GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, texture_format_cast(format), w, h);
  GL_CALL(glFramebufferRenderbuffer, fbbind, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbos[1]);

  GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, NULL_BINDING);

  NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);

  framebuffer_t fbo;
  fbo.id = id;
  fbo.sd_rbo = rbos[0];
  fbo.color_rbo = rbos[1];
  fbo.extent.x = w;
  fbo.extent.y = h;
  return fbo;
}

auto gl_state::create_framebuffer(uint32 w, uint32 h, r_test_buffer buffers,
                                  const fbo_attachment_t* attachments,
                                  uint32 att_count) -> framebuffer_t {
  NTF_ASSERT(att_count <= MAX_FBO_ATTACHMENTS);
  NTF_ASSERT(attachments);
  NTF_ASSERT(w > 0 && h > 0);
  const GLenum sd_attachment = fbo_attachment_cast(buffers);
  const GLenum fbbind = GL_DRAW_FRAMEBUFFER;

  GLuint id;
  GL_CALL(glGenFramebuffers, 1, &id);
  GL_CALL(glBindFramebuffer, fbbind, id);
  _bound_fbos[FBO_BIND_WRITE] = id;

  GLuint rbo{0};
  if (sd_attachment) {
    GL_CALL(glGenRenderbuffers, 1, &rbo);
    GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, rbo);
    GL_CALL(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    GL_CALL(glFramebufferRenderbuffer, fbbind, sd_attachment, GL_RENDERBUFFER, rbo);
    GL_CALL(glBindRenderbuffer, GL_RENDERBUFFER, NULL_BINDING);
  }

  for (uint32 i = 0; i < att_count; ++i) {
    NTF_ASSERT(attachments[i].tex);
    const auto& tex = *attachments[i].tex;
    bind_texture(tex.id, tex.type, _active_tex);
    switch (tex.type) {
      case GL_TEXTURE_1D: {
        NTF_ASSERT(w == tex.extent.x && h == 1);
        GL_CALL(glFramebufferTexture1D, fbbind, GL_COLOR_ATTACHMENT0+i,
                                        tex.type, tex.id, attachments[i].level);
        break;
      }
      case GL_TEXTURE_2D: {
        NTF_ASSERT(w == tex.extent.x && h == tex.extent.y);
        GL_CALL(glFramebufferTexture2D, fbbind, GL_COLOR_ATTACHMENT0+i,
                                        tex.type, tex.id, attachments[i].level);
        break;
      }
      default: {
        NTF_UNREACHABLE();
        break;
      }
    }
  }

  NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);

  framebuffer_t fbo;
  fbo.id = id;
  fbo.sd_rbo = rbo;
  fbo.color_rbo = NULL_BINDING;
  fbo.extent.x = w;
  fbo.extent.y = h;
  return fbo;
}

void gl_state::destroy_framebuffer(const framebuffer_t& fbo) noexcept {
  NTF_ASSERT(fbo.id);
  if (_bound_fbos[FBO_BIND_WRITE] == fbo.id) {
    GL_CALL(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, DEFAULT_FBO);
    _bound_fbos[FBO_BIND_WRITE] = DEFAULT_FBO;
  } else if (_bound_fbos[FBO_BIND_READ] == fbo.id) {
    GL_CALL(glBindFramebuffer, GL_READ_FRAMEBUFFER, DEFAULT_FBO);
    _bound_fbos[FBO_BIND_READ] = DEFAULT_FBO;
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

void gl_state::bind_framebuffer(GLuint id, fbo_binding binding) noexcept {
  GLenum fb;
  switch (binding) {
    case FBO_BIND_WRITE: {
      if (_bound_fbos[FBO_BIND_WRITE] == id) {
        return;
      }
      fb = GL_DRAW_FRAMEBUFFER;
      _bound_fbos[FBO_BIND_WRITE] = id;
      break;
    }
    case FBO_BIND_READ: {
      if (_bound_fbos[FBO_BIND_READ] == id) {
        return;
      }
      _bound_fbos[FBO_BIND_READ] = id;
      fb = GL_READ_FRAMEBUFFER;
      break;
    }
    case FBO_BIND_BOTH: {
      if (_bound_fbos[FBO_BIND_READ] == id && _bound_fbos[FBO_BIND_WRITE] == id) {
        return;
      }
      _bound_fbos[FBO_BIND_READ] = id;
      _bound_fbos[FBO_BIND_WRITE] = id;
      fb = GL_FRAMEBUFFER;
      break;
    }
    default: {
      NTF_UNREACHABLE();
    }
  }
  GL_CALL(glBindFramebuffer, fb, id);
}

} // namespace ntf
