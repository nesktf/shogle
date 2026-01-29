#pragma once

#include <shogle/render/gl/common.hpp>

namespace shogle {

class gl_texture {
public:
  using context_type = gl_context;

  enum texture_type : gldefs::GLenum {
    TEX_TYPE_1D = 0x0DE0,                   // GL_TEXTURE_1D
    TEX_TYPE_2D = 0x0DE1,                   // GL_TEXTURE_2D
    TEX_TYPE_3D = 0x806F,                   // GL_TEXTURE_3D
    TEX_TYPE_CUBEMAP = 0x8513,              // GL_TEXTURE_CUBEMAP
    TEX_TYPE_1D_ARRAY = 0x8C18,             // GL_TEXTURE_1D_ARRAY
    TEX_TYPE_2D_ARRAY = 0x8C1A,             // GL_TEXTURE_2D_ARRAY
    TEX_TYPE_BUFFER = 0x8C2A,               // GL_TEXTURE_BUFFER
    TEX_TYPE_2D_MULTISAMPLE_ARRAY = 0x9102, // GL_TEXTURE_2D_MULTISAMPLE_ARRAY
    TEX_TYPE_2D_MULTISAMPLE = 0x9100,       // GL_TEXTURE_2D_MULTISAMPLE
  };

  enum multisample_opt : gldefs::GLenum {
    MULTISAMPLE_NONE = 0x0000,
    MULTISAMPLE_NOT_FIXED = 0x0001, // GL_FALSE + 1
    MULTISAMPLE_FIXED = 0x0002,     // GL_TRUE + 1
  };

  enum texture_format : gldefs::GLenum {
    TEX_FORMAT_R8_UNORM = 0x8229, // GL_R8
    TEX_FORMAT_R8_SNORM = 0x8F94, // GL_R8_SNORM
    TEX_FORMAT_R8U = 0x8232,      // GL_R8UI
    TEX_FORMAT_R8I = 0x8231,      // GL_R8I

    TEX_FORMAT_RG8_UNORM = 0x822B, // GL_RG8
    TEX_FORMAT_RG8_SNORM = 0x8F95, // GL_RG8_SNORM
    TEX_FORMAT_RG8U = 0x8238,      // GL_RG8UI
    TEX_FORMAT_RG8I = 0x8239,      // GL_RG8I

    TEX_FORMAT_RGBA8_NORM = 0x8058, // GL_RGBA8

    TEX_FORMAT_D16 = 0x81A5,     // GL_DEPTH_COMPONENT16
    TEX_FORMAT_D24 = 0x81A6,     // GL_DEPTH_COMPONENT24
    TEX_FORMAT_D32 = 0x81A7,     // GL_DEPTH_COMPONENT32
    TEX_FORMAT_D32F = 0x8CAC,    // GL_DEPTH_COMPONENT32F
    TEX_FORMAT_D24_S8 = 0x88F0,  // GL_DEPTH24_STENCIL8
    TEX_FORMAT_D32F_S8 = 0x8CAD, // GL_DEPTH32F_STENCIL8

    TEX_FORMAT_DEPTH_COMPONENT = 0x1902, //  GL_DEPTH_COMPONENT
    TEX_FORMAT_DEPTH_STENCIL = 0x84F9,   // GL_DEPTH_STENCIL
    TEX_FORMAT_R = 0x1903,               // GL_RED
    TEX_FORMAT_RG = 0x8227,              // GL_RG
    TEX_FORMAT_RGB = 0x1907,             // GL_RGB
    TEX_FORMAT_RGBA = 0x1908,            // GL_RGBA
  };

  enum pixel_format : gldefs::GLenum {
    PIXEL_FORMAT_R_NORM = 0x1903,        // GL_RED
    PIXEL_FORMAT_RG_NORM = 0x8227,       // GL_RG
    PIXEL_FORMAT_RGB_NORM = 0x1907,      // GL_RGB
    PIXEL_FORMAT_BGR_NORM = 0x80E0,      // GL_BGR
    PIXEL_FORMAT_RGBA_NORM = 0x1908,     // GL_RGBA
    PIXEL_FORMAT_BGRA_NORM = 0x80E1,     // GL_BGRA
    PIXEL_FORMAT_DEPTH = 0x1902,         // GL_DEPTH_COMPONENT
    PIXEL_FORMAT_DEPTH_STENCIL = 0x84F9, // GL_DEPTH_STENCIL
  };

  enum pixel_data_type : gldefs::GLenum {
    PIXEL_TYPE_I8 = meta::gl_data_traits<i8>::gl_tag,   // GL_BYTE
    PIXEL_TYPE_U8 = meta::gl_data_traits<u8>::gl_tag,   // GL_UNSIGNED_BYTE
    PIXEL_TYPE_I16 = meta::gl_data_traits<i16>::gl_tag, // GL_SHORT
    PIXEL_TYPE_U16 = meta::gl_data_traits<u16>::gl_tag, // GL_UNSIGNED_SHORT
    PIXEL_TYPE_I32 = meta::gl_data_traits<i32>::gl_tag, // GL_INT
    PIXEL_TYPE_U32 = meta::gl_data_traits<u32>::gl_tag, // GL_UNSIGNED_INT
    PIXEL_TYPE_F32 = meta::gl_data_traits<f32>::gl_tag, // GL_FLOAT
    PIXEL_TYPE_F16 = 0x140B,                            // GL_HALF_FLOAT
    PIXEL_TYPE_U32D24S8 = 0x84FA,                       // GL_UNSIGNED_INT_24_8
  };

  enum texture_wrap : gldefs::GLenum {
    WRAP_CLAMP_TO_BORDER = 0x812D, // GL_CLAMP_TO_BORDER
    WRAP_CLAMP_TO_EDGE = 0x812F,   // GL_CLAMP_TO_EDGE
    WRAP_REPEAT = 0x2901,          // GL_REPEAT
    WRAP_MIRRORED_REPEAT = 0x8370, // GL_MIRRORED_REPEAT
  };

  enum wrap_direction : gldefs::GLenum {
    WRAP_DIR_S = 0x2802, // GL_TEXTURE_WRAP_S
    WRAP_DIR_T = 0x2803, // GL_TEXTURE_WRAP_T
    WRAP_DIR_R = 0x8072, // GL_TEXTURE_WRAP_R
  };

  enum texture_mag_sampler : gldefs::GLenum {
    SAMPLER_MAG_NEAREST = 0x2600, // GL_NEAREST
    SAMPLER_MAG_LINEAR = 0x2601,  // GL_LINEAR
  };

  enum texture_min_sampler : gldefs::GLenum {
    SAMPLER_MIN_NEAREST = 0x2600,            // GL_NEAREST
    SAMPLER_MIN_LINEAR = 0x2601,             // GL_LINEAR
    SAMPLER_MIN_NEAREST_MP_NEAREST = 0x2700, // GL_NEAREST_MIPMAP_NEAREST
    SAMPLER_MIN_LINEAR_MP_NEAREST = 0x2701,  // GL_LINEAR_MIPMAP_NEAREST
    SAMPLER_MIN_NEAREST_MP_LINEAR = 0x2702,  // GL_NEAREST_MIPMAP_LINEAR
    SAMPLER_MIN_LINEAR_MP_LINEAR = 0x2703,   // GL_LINEAR_MIPMAP_LINEAR
  };

  enum cubemap_face : gldefs::GLenum {
    CUBEMAP_POS_X = 0x8515, // GL_TEXTURE_CUBEMAP_POSITIVE_X
    CUBEMAP_NEG_X = 0x8516, // GL_TEXTURE_CUBEMAP_NEGATIVE_X
    CUBEMAP_POS_Y = 0x8517, // GL_TEXTURE_CUBEMAP_POSITIVE_Y
    CUBEMAP_NEG_Y = 0x8518, // GL_TEXTURE_CUBEMAP_NEGATIVE_Y
    CUBEMAP_POS_Z = 0x8519, // GL_TEXTURE_CUBEMAP_POSITIVE_Z
    CUBEMAP_NEG_Z = 0x851A, // GL_TEXTURE_CUBEMAP_NEGATIVE_Z
  };

  enum swizzle_target : gldefs::GLenum {
    SWIZZLE_TARGET_R = 0x8E42, // GL_TEXTURE_SWIZZLE_R
    SWIZZLE_TARGET_G = 0x8E43, // GL_TEXTURE_SWIZZLE_G
    SWIZZLE_TARGET_B = 0x8E44, // GL_TEXTURE_SWIZZLE_B
    SWIZZLE_TARGET_A = 0x8E45, // GL_TEXTURE_SWIZZLE_A
  };

  enum swizzle_mask : gldefs::GLenum {
    SWIZZLE_MASK_R = 0x1903, // GL_RED
    SWIZZLE_MASK_G = 0x1904, // GL_GREEN
    SWIZZLE_MASK_B = 0x1905, // GL_BLUE
    SWIZZLE_MASK_A = 0x1906, // GL_ALPHA
    SWIZZLE_MASK_ZERO = 0,   // GL_ZERO
    SWIZZLE_MASK_ONE = 1,    // GL_ONE
  };

  enum pixel_alignment : u32 {
    ALIGN_1BYTE = 1,
    ALIGN_2BYTES = 2,
    ALIGN_4BYTES = 4,
    ALIGN_8BYTES = 8,
  };

  struct image_data {
    const void* data;
    extent3d extent;
    pixel_format format;
    pixel_data_type datatype;
    pixel_alignment alignment;
  };

  static constexpr u32 MAX_MIPMAP_LEVEL = 7;

  struct cubemap_tag_t {};

  static constexpr cubemap_tag_t cubemap_tag{};

  struct allocate_args {
    extent3d extent;
    texture_type type;
    texture_format format;
    u32 levels;
    u32 layers;
    multisample_opt multisampling;
  };

  using n_err_return = std::pair<u32, gldefs::GLenum>;

private:
  struct create_t {};

  template<typename Cont>
  static constexpr bool emplace_tex_container =
    ntf::meta::growable_emplace_container_of<std::decay_t<Cont>, gl_texture, create_t,
                                             gldefs::GLhandle, const allocate_args&>;

  template<typename Cont>
  static constexpr bool growable_tex_container =
    ntf::meta::growable_push_container_of<std::decay_t<Cont>, gl_texture> ||
    emplace_tex_container<Cont>;

public:
  // Internal constructor
  explicit gl_texture(create_t, gldefs::GLhandle id, const allocate_args& args);

  // Internal buffer constructor
  explicit gl_texture(create_t, gldefs::GLhandle id, texture_format format, size_t size,
                      size_t offset);

  // TEX_TYPE_2D[_MULTISAMPLE/_ARRAY] constructor
  gl_texture(gl_context& gl, texture_format format, const extent2d& extent, u32 layers = 1,
             u32 levels = MAX_MIPMAP_LEVEL, multisample_opt multisampling = MULTISAMPLE_NONE);

  // TEX_TYPE_CUBEMAP constructor
  explicit gl_texture(cubemap_tag_t, gl_context& gl, texture_format format, u32 extent,
                      u32 levels = MAX_MIPMAP_LEVEL);

  // TEX_TYPE_1D[_MULTISAMPLE/_ARRAY] constructor
  explicit gl_texture(gl_context& gl, texture_format format, u32 extent, u32 layers = 1,
                      u32 levels = MAX_MIPMAP_LEVEL);

  // TEX_TYPE_3D constructor
  explicit gl_texture(gl_context& gl, texture_format format, const extent3d& extent,
                      u32 levels = MAX_MIPMAP_LEVEL);

  // TEX_TYPE_BUFFER constructor
  explicit gl_texture(gl_context& gl, const gl_buffer& buffer, texture_format format, size_t size,
                      size_t offset);

private:
  static n_err_return _allocate_span(gl_context& gl, span<gldefs::GLenum> texes,
                                     const allocate_args& args);

public:
  static gl_expect<gl_texture> allocate1d(gl_context& gl, texture_format format, u32 extent,
                                          u32 levels = MAX_MIPMAP_LEVEL, u32 layers = 1);
  template<typename Cont>
  static n_err_return allocate1d_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                                   u32 extent, u32 levels = MAX_MIPMAP_LEVEL, u32 layers = 1)
  requires(growable_tex_container<Cont>);

  static gl_expect<gl_texture> allocate2d(gl_context& gl, texture_format format,
                                          const extent2d& extent, u32 levels = MAX_MIPMAP_LEVEL,
                                          u32 layers = 1,
                                          multisample_opt multisampling = MULTISAMPLE_NONE);
  template<typename Cont>
  static n_err_return allocate2d_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                                   const extent2d& extent, u32 layers = 1,
                                   u32 levels = MAX_MIPMAP_LEVEL,
                                   multisample_opt multisampling = MULTISAMPLE_NONE)
  requires(growable_tex_container<Cont>);

  static gl_expect<gl_texture> allocate_cubemap(gl_context& gl, texture_format format, u32 extent,
                                                u32 levels = MAX_MIPMAP_LEVEL);

  template<typename Cont>
  static n_err_return allocate_cubemap_n(gl_context& gl, Cont&& cont, u32 count,
                                         texture_format format, u32 extent,
                                         u32 levels = MAX_MIPMAP_LEVEL)
  requires(growable_tex_container<Cont>);

  static gl_expect<gl_texture> allocate3d(gl_context& gl, texture_format format,
                                          const extent3d& extent, u32 levels = MAX_MIPMAP_LEVEL);

  template<typename Cont>
  static n_err_return allocate3d_n(gl_context& gl, Cont&& cont, u32 count, texture_format format,
                                   const extent3d& extent, u32 levels = MAX_MIPMAP_LEVEL)
  requires(growable_tex_container<Cont>);

  static gl_expect<gl_texture> bind_to_buffer(gl_context& gl, const gl_buffer& buffer,
                                              texture_format format, size_t size, size_t offset);

  static void deallocate(gl_context& gl, gl_texture& tex);
  static void deallocate_n(gl_context& gl, gl_texture* texes, u32 tex_count);
  static void deallocate_n(gl_context& gl, span<gl_texture> texes);

public:
  gl_expect<void> upload_image(gl_context& gl, const image_data& image,
                               const extent3d& offset = {}, u32 layer = 0, u32 level = 0);

  template<::shogle::meta::gl_data_type T, ::shogle::meta::extent_type Ext = extent2d>
  gl_expect<void> upload_image(gl_context& gl, span<const T> data, const Ext& extent,
                               pixel_format format, const Ext& offset = {}, u32 layer = 0,
                               u32 level = 0, pixel_alignment alignment = ALIGN_4BYTES);

  n_err_return upload_image_layers(gl_context& gl, span<const image_data> layers,
                                   const extent3d& offset = {}, u32 level = 0);

  n_err_return upload_image_layers(gl_context& gl, const image_data* layers, u32 layer_count,
                                   const extent3d& offset = {}, u32 level = 0);

  void generate_mipmaps(gl_context& gl);

public:
  gl_texture& set_swizzle(gl_context& gl, swizzle_target target, swizzle_mask mask);
  gl_texture& set_wrap(gl_context& gl, wrap_direction dir, texture_wrap wrap);
  gl_texture& set_sampler_mag(gl_context& gl, texture_mag_sampler sampler);
  gl_texture& set_sampler_min(gl_context& gl, texture_min_sampler sampler);

public:
  gldefs::GLhandle id() const;
  extent3d extent() const;
  texture_type type() const;
  texture_format format() const;
  u32 layers() const;
  u32 levels() const;
  size_t buffer_size() const;
  size_t buffer_offset() const;
  bool invalidated() const;

public:
  explicit operator bool() const { return !invalidated(); }

private:
  extent3d _extent;
  u32 _layers;
  u32 _levels;
  gldefs::GLhandle _id;
  texture_type _type;
  texture_format _format;
};

static_assert(meta::renderer_object_type<gl_texture>);

} // namespace shogle

#ifndef SHOGLE_GL_TEXTURE_INL
#include <shogle/render/gl/texture.inl>
#endif
