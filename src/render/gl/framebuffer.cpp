#include "./context_private.hpp"
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

u32 attach_colors([[maybe_unused]] gl_context& gl, [[maybe_unused]] extent2d extent,
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

} // namespace

gl_sv_expect<gl_framebuffer>
gl_framebuffer::from_renderbuffer(gl_context& gl, extent2d extent,
                                  span<const texture_attachment> color,
                                  const gl_renderbuffer& rbo) {
  const auto find_rbo_attachment = [](gl_renderbuffer::buffer_format format) -> GLenum {
    switch (format) {
      case gl_renderbuffer::FORMAT_D24u_S8u:
        [[fallthrough]];
      case gl_renderbuffer::FORMAT_D32f_S8u:
        return GL_DEPTH_STENCIL_ATTACHMENT;
      case gl_renderbuffer::FORMAT_D16u:
        [[fallthrough]];
      case gl_renderbuffer::FORMAT_D24u:
        [[fallthrough]];
      case gl_renderbuffer::FORMAT_D32u:
        [[fallthrough]];
      case gl_renderbuffer::FORMAT_D32f:
        return GL_DEPTH_ATTACHMENT;
      case gl_renderbuffer::FORMAT_S8u:
        return GL_STENCIL_ATTACHMENT;
      default:
        break;
    };
    SHOGLE_UNREACHABLE();
  };
  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, rbo.id()));
  auto err = GL_RET_ERR(glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, find_rbo_attachment(rbo.format()), GL_RENDERBUFFER, rbo.id()));
  GL_ASSERT(glBindRenderbuffer(GL_RENDERBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind renderbuffer", err};
  }

  const u32 attachment_count = attach_colors(gl, extent, color);
  err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Incomplete framebuffer", err};
  }

  return {in_place, create_t{}, fbo, attachment_count, BUFFER_ATT_RENDERBUFFER, extent};
}

gl_sv_expect<gl_framebuffer> gl_framebuffer::from_texture(gl_context& gl, extent2d extent,
                                                          span<const texture_attachment> color,
                                                          const texture_attachment& tex) {
  const auto find_tex_component = [](gl_texture::texture_format format) -> GLenum {
    // TODO: fill this thing
    switch (format) {
      case GL_DEPTH_COMPONENT:
        return GL_DEPTH_ATTACHMENT;
      default:
        break;
    }
    return 0;
  };
  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

  const u32 level = tex.level;
  const u32 layer = tex.layer;
  const gl_texture& texture = tex.tex.get();
  const auto attachment_component = find_tex_component(texture.format());
  if (!attachment_component) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Invalid buffer texture format", GL_INVALID_VALUE};
  }
  const auto [twidth, theight, _] = texture.extent();
  const auto type = texture.type();
  GL_ASSERT(glBindTexture(type, texture.id()));
  GLenum err = GL_NO_ERROR;
  switch (type) {
    case gl_texture::TEX_TYPE_1D: {
      SHOGLE_ASSERT(twidth == extent.width, "Buffer texture width mismatch");
      err = GL_RET_ERR(glFramebufferTexture1D(GL_FRAMEBUFFER, attachment_component, GL_TEXTURE_1D,
                                              texture.id(), level));
    } break;
    case gl_texture::TEX_TYPE_2D: {
      SHOGLE_ASSERT(twidth == extent.width, "Buffer texture width mismatch");
      SHOGLE_ASSERT(theight == extent.height, "Buffer texture height mismatch");
      err = GL_RET_ERR(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_component, GL_TEXTURE_2D,
                                              texture.id(), level));
    } break;
    case gl_texture::TEX_TYPE_CUBEMAP:
      [[fallthrough]];
    case gl_texture::TEX_TYPE_3D: {
      SHOGLE_ASSERT(twidth == extent.width, "Buffer texture width mismatch");
      SHOGLE_ASSERT(theight == extent.height, "Buffer texture height mismatch");
      err = GL_RET_ERR(glFramebufferTexture3D(GL_FRAMEBUFFER, attachment_component, type,
                                              texture.id(), level, layer));
    } break;
    default: {
      err = GL_INVALID_VALUE;
    } break;
  }
  GL_ASSERT(glBindTexture(type, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Failed to bind texture buffer", err};
  }

  const u32 attachment_count = attach_colors(gl, extent, color);
  err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glDeleteFramebuffers(1, &fbo));
    return {unexpect, "Incomplete framebuffer", err};
  }

  return {in_place, create_t{}, fbo, attachment_count, BUFFER_ATT_TEXTURE, extent};
}

gl_sv_expect<gl_framebuffer>
gl_framebuffer::from_color_only(gl_context& gl, extent2d extent,
                                span<const texture_attachment> color) {
  GLuint fbo;
  GL_ASSERT(glGenFramebuffers(1, &fbo));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, fbo));
  const u32 attachment_count = attach_colors(gl, extent, color);
  const auto err = GL_CALL(glCheckFramebufferStatus(GL_FRAMEBUFFER));
  GL_ASSERT(glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_BINDING));
  if (err) {
    return {unexpect, "Incomplete framebuffer", err};
  }
  SHOGLE_ASSERT(attachment_count);
  return {in_place, create_t{}, fbo, attachment_count, BUFFER_ATT_NONE, extent};
}

gl_framebuffer::gl_framebuffer(create_t, gldefs::GLhandle fbo, u32 color_attachments,
                               buffer_attachment attachment, extent2d extent) :
    _extent(extent), _id(fbo), _buffer_attachment(attachment), _color_count(color_attachments) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent,
                               span<const texture_attachment> color, const gl_renderbuffer& rbo) :
    gl_framebuffer(::shogle::gl_framebuffer::from_renderbuffer(gl, extent, color, rbo).value()) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent,
                               span<const texture_attachment> color,
                               const texture_attachment& tex) :
    gl_framebuffer(::shogle::gl_framebuffer::from_texture(gl, extent, color, tex).value()) {}

gl_framebuffer::gl_framebuffer(gl_context& gl, extent2d extent,
                               span<const texture_attachment> color) :
    gl_framebuffer(::shogle::gl_framebuffer::from_color_only(gl, extent, color).value()) {}

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

gl_expect<void> gl_framebuffer::blit(gl_context& gl, const gl_framebuffer& source,
                                     const rectangle_pos<u32>& source_area,
                                     const gl_framebuffer& dest,
                                     const rectangle_pos<u32>& dest_area,
                                     gl_framebuffer::buffer_target target_mask,
                                     gl_framebuffer::buffer_filter filter) {
  const auto [src_x, src_y, src_w, src_h] = source_area;
  const auto [dst_x, dst_y, dst_w, dst_h] = dest_area;
  GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.id()));
  GL_ASSERT(glBindFramebuffer(GL_READ_FRAMEBUFFER, source.id()));
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

gl_framebuffer::buffer_attachment gl_framebuffer::attachment_type() const {
  SHOGLE_ASSERT(!invalidated(), "gl_framebuffer use after free");
  return _buffer_attachment;
}

u32 gl_framebuffer::color_attachment_count() const {
  SHOGLE_ASSERT(!invalidated(), "gl_framebuffer use after free");
  return _color_count;
}

bool gl_framebuffer::invalidated() const noexcept {
  return _id == GL_NULL_HANDLE;
}

} // namespace shogle
