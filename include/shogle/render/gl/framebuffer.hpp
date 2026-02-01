#pragma once

#include <shogle/render/gl/texture.hpp>

namespace shogle {

class gl_renderbuffer {
public:
  enum buffer_format : gldefs::GLenum {
    FORMAT_D24u_S8u = 0x88F0, // GL_DEPTH24_STENCIL8,
    FORMAT_D32f_S8u = 0x8CAD, // GL_DEPTH32F_STENCIL8,
    FORMAT_D16u = 0x81A5,     // GL_DEPTH_COMPONENT16,
    FORMAT_D24u = 0x81A6,     // GL_DEPTH_COMPONENT24,
    FORMAT_D32u = 0x81A7,     // GL_DEPTH_COMPONENT32,
    FORMAT_D32f = 0x8CAC,     // GL_DEPTH_COMPONENT32F,
    FORMAT_S8u = 0x8D48,      // GL_STENCIL_INDEX8,
  };

private:
  struct create_t {};

public:
  gl_renderbuffer(create_t, gldefs::GLhandle id, buffer_format format, extent2d extent);
  gl_renderbuffer(gl_context& gl, buffer_format format, extent2d extent);

public:
  static gl_sv_expect<gl_renderbuffer> create(gl_context& gl, buffer_format format,
                                              extent2d extent);

  static void destroy(gl_context& gl, gl_renderbuffer& rbo);

public:
  gldefs::GLhandle id() const;
  buffer_format format() const;
  extent2d extent() const;

private:
  extent2d _extent;
  gldefs::GLhandle _id;
  buffer_format _format;
};

class gl_framebuffer {
public:
  struct texture_attachment {
    ref_view<const gl_texture> tex;
    u32 level;
    u32 layer;
  };

  enum buffer_attachment : gldefs::GLenum {
    BUFFER_ATT_NONE = 0,
    BUFFER_ATT_TEXTURE,
    BUFFER_ATT_RENDERBUFFER,
  };

  enum buffer_target : gldefs::GLbitfield {
    FORMAT_COLOR_BIT = 0x00004000,   // GL_COLOR_FORMAT_BIT
    FORMAT_DEPTH_BIT = 0x00000100,   // GL_DEPTH_FORMAT_BIT,
    FORMAT_STENCIL_BIT = 0x00000400, // GL_STENCIL_FORMAT_BIT,
  };

  enum buffer_filter : gldefs::GLenum {
    FORMAT_FILTER_NEAREST = 0x2600, // GL_NEAREST
    FORMAT_FILTER_LINEAR = 0x2601,  // GL_LINEAR,
  };

private:
  struct create_t {};

public:
  gl_framebuffer(create_t, gldefs::GLhandle fbo, u32 color_attachments,
                 buffer_attachment attachment, extent2d extent);

  explicit gl_framebuffer(gl_context& gl, extent2d extent, span<const texture_attachment> color,
                          const gl_renderbuffer& rbo);

  explicit gl_framebuffer(gl_context& gl, extent2d extent, span<const texture_attachment> color,
                          const texture_attachment& tex);

  explicit gl_framebuffer(gl_context& gl, extent2d extent, span<const texture_attachment> color);

public:
  static gl_sv_expect<gl_framebuffer> from_renderbuffer(gl_context& gl, extent2d extent,
                                                        span<const texture_attachment> color,
                                                        const gl_renderbuffer& rbo);

  static gl_sv_expect<gl_framebuffer> from_texture(gl_context& gl, extent2d extent,
                                                   span<const texture_attachment> color,
                                                   const texture_attachment& tex);

  static gl_sv_expect<gl_framebuffer> from_color_only(gl_context& gl, extent2d extent,
                                                      span<const texture_attachment> color);

  static void destroy(gl_context& gl);

public:
  static gl_expect<void> blit(gl_context& gl, const gl_framebuffer& source,
                              const square_pos<u32>& source_area, const gl_framebuffer& dest,
                              const square_pos<u32>& dest_area,
                              gl_framebuffer::buffer_target target_mask,
                              gl_framebuffer::buffer_filter filter);

public:
  gldefs::GLhandle id() const;
  extent2d extent() const;
  buffer_attachment attachment_type() const;
  u32 color_attachment_count() const;

private:
  extent2d _extent;
  gldefs::GLhandle _id;
  buffer_attachment _buffer_attachment;
  u32 _color_count;
};

} // namespace shogle
