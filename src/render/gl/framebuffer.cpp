#include "./context_private.hpp"
#include <GL/glext.h>
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/framebuffer.hpp>

namespace shogle {

gl_sv_expect<gl_renderbuffer> gl_renderbuffer::create(gl_context& gl, buffer_format format,
                                                      extent2d extent) {
  GLuint rbo;
  GL_ASSERT(glGenRenderbuffers(1, &rbo));
  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
  const auto err =
    GL_RET_ERR(glRenderbufferStorage(GL_RENDERBUFFER, format, extent.width, extent.height));
  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glDeleteRenderbuffers(1, &rbo));
    return {unexpect, "Failed to create renderbuffer", err};
  }
  return {in_place, create_t{}, rbo, format, extent};
}

gl_renderbuffer::gl_renderbuffer(create_t, gldefs::GLhandle id, buffer_format format,
                                 extent2d extent) : _extent(extent), _id(id), _format(format) {}

gl_renderbuffer::gl_renderbuffer(gl_context& gl, buffer_format format, extent2d extent) :
    gl_renderbuffer(::shogle::gl_renderbuffer::create(gl, format, extent).value()) {}

void gl_renderbuffer::destroy(gl_context& gl, gl_renderbuffer& rbo) noexcept {
  if (SHOGLE_UNLIKELY(rbo.invalidated())) {
    return;
  }
  GL_ASSERT(glDeleteRenderbuffers(1, &rbo._id));
  rbo._id = GL_NULL_HANDLE;
}

void gl_renderbuffer::destroy_n(gl_context& gl, gl_renderbuffer* rbos, size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!rbos)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(rbos[i].invalidated())) {
      continue;
    }
    GL_CALL(glDeleteRenderbuffers(1, &rbos[i]._id));
    rbos[i]._id = GL_NULL_HANDLE;
  }
}

void gl_renderbuffer::destroy_n(gl_context& gl, span<gl_renderbuffer> rbos) noexcept {
  destroy_n(gl, rbos.data(), rbos.size());
}

gldefs::GLhandle gl_renderbuffer::id() const {
  SHOGLE_ASSERT(!invalidated(), "gl_renderbuffer use after free");
  return _id;
}

gl_renderbuffer::buffer_format gl_renderbuffer::format() const {
  SHOGLE_ASSERT(!invalidated(), "gl_renderbuffer use after free");
  return _format;
}

extent2d gl_renderbuffer::extent() const {
  SHOGLE_ASSERT(!invalidated(), "gl_renderbuffer use after free");
  return _extent;
}

bool gl_renderbuffer::invalidated() const noexcept {
  return _id == GL_NULL_HANDLE;
}

namespace {

u32 attach_colors(gl_context& gl, [[maybe_unused]] extent2d extent,
                  span<const gl_framebuffer::texture_attachment> attachments) {
  u32 attachment_count = 0;
  for (const auto& [texture, level, layer] : attachments) {
    const gl_texture& tex = texture.get();
    const auto [twidth, theight, _] = tex.extent();
    const auto type = tex.type();
    GL_ASSERT(glBindTexture(type, tex.id()));
    switch (type) {
      case GL_TEXTURE_1D: {
        SHOGLE_ASSERT(twidth == extent.width, "Color texture width mismatch");
        GL_ASSERT(glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_count,
                                         GL_TEXTURE_1D, tex.id(), level));
      } break;
      case GL_TEXTURE_2D: {
        SHOGLE_ASSERT(twidth == extent.width, "Color texture width mismatch");
        SHOGLE_ASSERT(theight == extent.height, "Color texture height mismatch");
        GL_ASSERT(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_count,
                                         GL_TEXTURE_2D, tex.id(), level));
      } break;
      case GL_TEXTURE_CUBE_MAP:
        [[fallthrough]];
      case GL_TEXTURE_3D: {
        SHOGLE_ASSERT(twidth == extent.width, "Color texture width mismatch");
        SHOGLE_ASSERT(theight == extent.height, "Color texture height mismatch");
        GL_ASSERT(glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_count,
                                         type, tex.id(), level, layer));
      } break;
      default:
        continue;
    }
    GL_ASSERT(glBindTexture(type, GL_DEFAULT_BINDING));
    ++attachment_count;
  }
  return attachment_count;
}

gldefs::GLenum find_rbo_attachment(gl_renderbuffer::buffer_format format) {
  switch (format) {
    case gl_renderbuffer::FORMAT_D24U_S8U:
      [[fallthrough]];
    case gl_renderbuffer::FORMAT_D32F_S8U:
      return GL_DEPTH_STENCIL_ATTACHMENT;
    case gl_renderbuffer::FORMAT_D16U:
      [[fallthrough]];
    case gl_renderbuffer::FORMAT_D24U:
      [[fallthrough]];
    case gl_renderbuffer::FORMAT_D32U:
      [[fallthrough]];
    case gl_renderbuffer::FORMAT_D32F:
      return GL_DEPTH_ATTACHMENT;
    case gl_renderbuffer::FORMAT_S8U:
      return GL_STENCIL_ATTACHMENT;
    default:
      return 0;
  };
  SHOGLE_UNREACHABLE();
}

GLenum attach_tex_buffer(gl_context& gl, const gl_framebuffer::texture_attachment& buffer,
                         extent2d extent, gldefs::GLenum attachment) {
  switch (buffer.tex->type()) {
    case gl_texture::TEX_TYPE_1D: {
      return GL_RET_ERR(glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_1D,
                                               buffer.tex->id(), buffer.level));
    } break;
    case gl_texture::TEX_TYPE_2D: {
      return GL_RET_ERR(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D,
                                               buffer.tex->id(), buffer.level));
    } break;
    case gl_texture::TEX_TYPE_CUBEMAP:
      [[fallthrough]];
    case gl_texture::TEX_TYPE_3D: {
      return GL_RET_ERR(glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, buffer.tex->type(),
                                               buffer.tex->id(), buffer.level, buffer.layer));
    } break;
    default: {
      return GL_INVALID_VALUE;
    } break;
  }
}

} // namespace

gl_sv_expect<gl_framebuffer> gl_framebuffer::with_textures(gl_context& gl, extent2d extent,
                                                           span<const texture_attachment> color,
                                                           const gl_renderbuffer& buffer) {
  const auto attachment = find_rbo_attachment(buffer.format());
  if (!attachment) {
    return {unexpect, "Invalid renderbuffer attachment format"};
  }
  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, buffer.id()));
  auto err = GL_RET_ERR(
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer.id()));
  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind renderbuffer", err};
  }

  attach_colors(gl, extent, color);
  err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err != GL_FRAMEBUFFER_COMPLETE) {
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Incomplete framebuffer", err};
  }

  SHOGLE_GL_LOG(VERBOSE,
                "FRAMEBUFFER_CREATE ({}) [ext: {}x{}, col_att: {} (tex), buff_att: {} (rbo)]", fbo,
                extent.width, extent.height, color.size(), buffer.id());

  return {in_place, create_t{}, fbo, extent};
}

gl_sv_expect<gl_framebuffer> gl_framebuffer::with_textures(gl_context& gl, extent2d extent,
                                                           span<const texture_attachment> color,
                                                           const texture_attachment& buffer) {
  const auto format = buffer.tex->format();
  if (format != gl_texture::TEX_FORMAT_DEPTH_STENCIL || gl_texture::TEX_FORMAT_DEPTH_COMPONENT) {
    return {unexpect, "Invalid buffer texture attachment format"};
  }
  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  const auto attachment = format == gl_texture::TEX_FORMAT_DEPTH_STENCIL
                          ? GL_DEPTH_STENCIL_ATTACHMENT
                          : GL_DEPTH_ATTACHMENT;
  const auto [twidth, theight, _] = buffer.tex->extent();
  const auto type = buffer.tex->type();
  GL_ASSERT(glBindTexture(type, buffer.tex->id()));
  GLenum err = attach_tex_buffer(gl, buffer, extent, attachment);
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind texture buffer", err};
  }

  attach_colors(gl, extent, color);
  err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err != GL_FRAMEBUFFER_COMPLETE) {
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Incomplete framebuffer", err};
  }

  SHOGLE_GL_LOG(VERBOSE,
                "FRAMEBUFFER_CREATE ({}) [ext: {}x{}, col_att: {} (tex), buff_att: {} (tex)]", fbo,
                extent.width, extent.height, color.size(), buffer.tex->id());

  return {in_place, create_t{}, fbo, extent};
}

gl_sv_expect<gl_framebuffer> gl_framebuffer::with_renderbuffer(gl_context& gl, extent2d extent,
                                                               const gl_renderbuffer& color,
                                                               const gl_renderbuffer& buffer) {
  const auto attachment = find_rbo_attachment(buffer.format());
  if (!attachment) {
    return {unexpect, "Invalid renderbuffer attachment format"};
  }
  if (color.format() != gl_renderbuffer::FORMAT_SRGB8_A8 ||
      color.format() != gl_renderbuffer::FORMAT_RGB10_A2UI) {
    return {unexpect, "Invalid color attachment format"};
  }
  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, buffer.id()));
  auto err = GL_RET_ERR(
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer.id()));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind buffer renderbuffer", err};
  }

  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, color.id()));
  err = GL_RET_ERR(
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color.id()));
  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind buffer renderbuffer", err};
  }

  err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err != GL_FRAMEBUFFER_COMPLETE) {
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Incomplete framebuffer", err};
  }
  SHOGLE_GL_LOG(VERBOSE,
                "FRAMEBUFFER_CREATE ({}) [ext: {}x{}, col_att: 1 (rbo), buff_att: {} (rbo)]", fbo,
                extent.width, extent.height, buffer.id());

  return {in_place, create_t{}, fbo, extent};
}

gl_sv_expect<gl_framebuffer> gl_framebuffer::with_renderbuffer(gl_context& gl, extent2d extent,
                                                               const gl_renderbuffer& color,
                                                               const texture_attachment& buffer) {
  const auto format = buffer.tex->format();
  if (format != gl_texture::TEX_FORMAT_DEPTH_STENCIL || gl_texture::TEX_FORMAT_DEPTH_COMPONENT) {
    return {unexpect, "Invalid buffer texture attachment format"};
  }
  if (color.format() != gl_renderbuffer::FORMAT_SRGB8_A8 ||
      color.format() != gl_renderbuffer::FORMAT_RGB10_A2UI) {
    return {unexpect, "Invalid color attachment format"};
  }

  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  const u32 level = buffer.level;
  const u32 layer = buffer.layer;
  const auto attachment = format == gl_texture::TEX_FORMAT_DEPTH_STENCIL
                          ? GL_DEPTH_STENCIL_ATTACHMENT
                          : GL_DEPTH_ATTACHMENT;
  const auto [twidth, theight, _] = buffer.tex->extent();
  const auto type = buffer.tex->type();
  GL_ASSERT(glBindTexture(type, buffer.tex->id()));
  GLenum err = attach_tex_buffer(gl, buffer, extent, attachment);
  GL_ASSERT(glBindTexture(type, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind texture buffer", err};
  }

  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, color.id()));
  err = GL_RET_ERR(
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color.id()));
  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind buffer renderbuffer", err};
  }

  err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err != GL_FRAMEBUFFER_COMPLETE) {
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Incomplete framebuffer", err};
  }

  SHOGLE_GL_LOG(VERBOSE,
                "FRAMEBUFFER_CREATE ({}) [ext: {}x{}, col_att: 1 (rbo), buff_att: {} (tex)]", fbo,
                extent.width, extent.height, buffer.tex->id());

  return {in_place, create_t{}, fbo, extent};
}

gl_framebuffer::gl_framebuffer(create_t, gldefs::GLhandle fbo, extent2d extent) noexcept :
    _extent(extent), _id(fbo) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent,
                               span<const texture_attachment> color,
                               const gl_renderbuffer& buffer) :
    gl_framebuffer(::shogle::gl_framebuffer::with_textures(gl, extent, color, buffer).value()) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent,
                               span<const texture_attachment> color,
                               const texture_attachment& buffer) :
    gl_framebuffer(::shogle::gl_framebuffer::with_textures(gl, extent, color, buffer).value()) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent, const gl_renderbuffer& color,
                               const gl_renderbuffer& buffer) :
    gl_framebuffer(
      ::shogle::gl_framebuffer::with_renderbuffer(gl, extent, color, buffer).value()) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent, const gl_renderbuffer& color,
                               const texture_attachment& buffer) :
    gl_framebuffer(
      ::shogle::gl_framebuffer::with_renderbuffer(gl, extent, color, buffer).value()) {}

void gl_framebuffer::destroy(gl_context& gl, gl_framebuffer& fbo) noexcept {
  if (SHOGLE_UNLIKELY(fbo.invalidated())) {
    return;
  }
  GL_CALL(glDeleteFramebuffers(1, &fbo._id));
  fbo._id = GL_NULL_HANDLE;
}

void gl_framebuffer::destroy_n(gl_context& gl, gl_framebuffer* fbos, size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!fbos)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(fbos[i].invalidated())) {
      continue;
    }
    GL_CALL(glDeleteFramebuffers(1, &fbos[i]._id));
    fbos[i]._id = GL_NULL_HANDLE;
  }
}

void gl_framebuffer::destroy_n(gl_context& gl, span<gl_framebuffer> fbos) noexcept {
  destroy_n(gl, fbos.data(), fbos.size());
}

gl_expect<void> gl_framebuffer::blit(gl_context& gl, gldefs::GLhandle src_fbo,
                                     const rectangle_pos<u32>& src_area, gldefs::GLhandle dst_fbo,
                                     const rectangle_pos<u32>& dest_area,
                                     gl_framebuffer::buffer_target target_mask,
                                     gl_framebuffer::buffer_filter filter) {
  const auto [src_x, src_y, src_w, src_h] = src_area;
  const auto [dst_x, dst_y, dst_w, dst_h] = dest_area;
  GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo));
  GL_ASSERT(glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo));
  const auto err = GL_RET_ERR(glBlitFramebuffer(src_x, src_y, src_w, src_h, dst_x, dst_y, dst_w,
                                                dst_h, target_mask, filter));
  GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_DEFAULT_BINDING));
  GL_ASSERT(glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err != GL_NO_ERROR) {
    return {unexpect, err};
  }
  return {};
}

gldefs::GLhandle gl_framebuffer::id() const {
  SHOGLE_ASSERT(!invalidated(), "gl_framebuffer use after free");
  return _id;
}

extent2d gl_framebuffer::extent() const {
  SHOGLE_ASSERT(!invalidated(), "gl_framebuffer use after free");
  return _extent;
}

bool gl_framebuffer::invalidated() const noexcept {
  return _id == GL_NULL_HANDLE;
}

} // namespace shogle
