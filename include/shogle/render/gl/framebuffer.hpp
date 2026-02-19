#pragma once

#include <shogle/render/gl/texture.hpp>

namespace shogle {

class gl_renderbuffer {
public:
  using context_type = gl_context;
  using deleter_type = gl_deleter<gl_renderbuffer>;

public:
  enum buffer_format : gldefs::GLenum {
    FORMAT_RGB10_A2UI = 0x8059, // GL_RGBA10_A2UI
    FORMAT_SRGB8_A8 = 0x8C43,   // GL_SRGB8_ALPHA8
    FORMAT_D24U_S8U = 0x88F0,   // GL_DEPTH24_STENCIL8,
    FORMAT_D32F_S8U = 0x8CAD,   // GL_DEPTH32F_STENCIL8,
    FORMAT_D16U = 0x81A5,       // GL_DEPTH_COMPONENT16,
    FORMAT_D24U = 0x81A6,       // GL_DEPTH_COMPONENT24,
    FORMAT_D32U = 0x81A7,       // GL_DEPTH_COMPONENT32,
    FORMAT_D32F = 0x8CAC,       // GL_DEPTH_COMPONENT32F,
    FORMAT_S8U = 0x8D48,        // GL_STENCIL_INDEX8,
  };

private:
  struct create_t {};

public:
  gl_renderbuffer(create_t, gldefs::GLhandle id, buffer_format format, extent2d extent);
  gl_renderbuffer(gl_context& gl, buffer_format format, extent2d extent);

public:
  static gl_sv_expect<gl_renderbuffer> create(gl_context& gl, buffer_format format,
                                              extent2d extent);

  static void destroy(gl_context& gl, gl_renderbuffer& rbo) noexcept;
  static void destroy_n(gl_context& gl, gl_renderbuffer* rbos, size_t count) noexcept;
  static void destroy_n(gl_context& gl, span<gl_renderbuffer> rbos) noexcept;

public:
  gldefs::GLhandle id() const;
  buffer_format format() const;
  extent2d extent() const;

  bool invalidated() const noexcept;

public:
  explicit operator bool() const noexcept { return !invalidated(); }

private:
  extent2d _extent;
  gldefs::GLhandle _id;
  buffer_format _format;
};

static_assert(::shogle::meta::renderer_object_type<gl_renderbuffer>);

template<>
struct gl_deleter<gl_renderbuffer> {
public:
  gl_deleter(gl_context& gl) noexcept : _gl(&gl) {}

public:
  void operator()(gl_renderbuffer* rbos, size_t count) const noexcept {
    gl_renderbuffer::destroy_n(*_gl, rbos, count);
  }

  void operator()(gl_renderbuffer& rbo) const noexcept { gl_renderbuffer::destroy(*_gl, rbo); }

private:
  gl_context* _gl;
};

class gl_framebuffer {
public:
  struct texture_attachment {
    ref_view<const gl_texture> tex;
    u32 level;
    u32 layer;
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
  gl_framebuffer(create_t, gldefs::GLhandle fbo, extent2d extent) noexcept;

  gl_framebuffer(gl_context& gl, extent2d extent, span<const texture_attachment> color,
                 const gl_renderbuffer& buffer);

  explicit gl_framebuffer(gl_context& gl, extent2d extent, span<const texture_attachment> color,
                          const texture_attachment& buffer);

  explicit gl_framebuffer(gl_context& gl, extent2d extent, const gl_renderbuffer& color,
                          const gl_renderbuffer& buffer);

  explicit gl_framebuffer(gl_context& gl, extent2d extent, const gl_renderbuffer& color,
                          const texture_attachment& buffer);

public:
  static gl_sv_expect<gl_framebuffer> with_textures(gl_context& gl, extent2d extent,
                                                    span<const texture_attachment> color,
                                                    const gl_renderbuffer& buffer);

  static gl_sv_expect<gl_framebuffer> with_textures(gl_context& gl, extent2d extent,
                                                    span<const texture_attachment> color,
                                                    const texture_attachment& buffer);

  static gl_sv_expect<gl_framebuffer> with_renderbuffer(gl_context& gl, extent2d extent,
                                                        const gl_renderbuffer& color,
                                                        const gl_renderbuffer& buffer);

  static gl_sv_expect<gl_framebuffer> with_renderbuffer(gl_context& gl, extent2d extent,
                                                        const gl_renderbuffer& color,
                                                        const texture_attachment& buffer);

  static void destroy(gl_context& gl, gl_framebuffer& fbo) noexcept;

  static void destroy_n(gl_context& gl, gl_framebuffer* fbos, size_t count) noexcept;

  static void destroy_n(gl_context& gl, span<gl_framebuffer> fbos) noexcept;

public:
  static gl_expect<void> blit(gl_context& gl, gldefs::GLhandle src_fbo,
                              const rectangle_pos<u32>& src_area, gldefs::GLhandle dst_fbo,
                              const rectangle_pos<u32>& dest_area,
                              gl_framebuffer::buffer_target target_mask,
                              gl_framebuffer::buffer_filter filter);

public:
  gldefs::GLhandle id() const;
  extent2d extent() const;

  bool invalidated() const noexcept;

public:
  explicit operator bool() const noexcept { return !invalidated(); }

private:
  extent2d _extent;
  gldefs::GLhandle _id;
};

template<>
struct gl_deleter<gl_framebuffer> {
public:
  gl_deleter(gl_context& gl) noexcept : _gl(&gl) {}

public:
  void operator()(gl_framebuffer* fbos, size_t count) const noexcept {
    gl_framebuffer::destroy_n(*_gl, fbos, count);
  }

  void operator()(gl_framebuffer& fbo) const noexcept { gl_framebuffer::destroy(*_gl, fbo); }

private:
  gl_context* _gl;
};

} // namespace shogle
