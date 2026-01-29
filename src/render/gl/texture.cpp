#include "./context_private.hpp"
#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/texture.hpp>

#include <ntfstl/logger.hpp>

namespace shogle {

// Internal constructor
gl_texture::gl_texture(create_t, gldefs::GLhandle id, const allocate_args& args) :
    _extent(args.extent), _layers(args.layers), _levels(args.levels), _id(id), _type(args.type),
    _format(args.format) {}

// Internal buffer constructor
gl_texture::gl_texture(create_t, gldefs::GLhandle id, texture_format format, size_t size,
                       size_t offset) :
    // HACK: store the u64 size and offset inside extent, layers and levels
    _extent(static_cast<u32>(size), static_cast<u32>(size >> 32), 0),
    _layers(static_cast<u32>(offset)), _levels(static_cast<u32>(offset >> 32)), _id(id),
    _type(TEX_TYPE_BUFFER), _format(format) {}

// TEX_TYPE_2D[_MULTISAMPLE/_ARRAY] constructor
gl_texture::gl_texture(gl_context& gl, texture_format format, const extent2d& extent, u32 layers,
                       u32 levels, multisample_opt multisampling) :
    gl_texture(::shogle::gl_texture::allocate2d(gl, format, extent, layers, levels, multisampling)
                 .value()) {}

// TEX_TYPE_CUBEMAP constructor
gl_texture::gl_texture(cubemap_tag_t, gl_context& gl, texture_format format, u32 extent,
                       u32 levels) :
    gl_texture(::shogle::gl_texture::allocate_cubemap(gl, format, extent, levels).value()) {}

// TEX_TYPE_1D[_MULTISAMPLE/_ARRAY] constructor
gl_texture::gl_texture(gl_context& gl, texture_format format, u32 extent, u32 layers, u32 levels) :
    gl_texture(::shogle::gl_texture::allocate1d(gl, format, extent, layers, levels).value()) {}

// TEX_TYPE_3D constructor
gl_texture::gl_texture(gl_context& gl, texture_format format, const extent3d& extent, u32 levels) :
    gl_texture(::shogle::gl_texture::allocate3d(gl, format, extent, levels).value()) {}

// TEX_TYPE_BUFFER constructor
gl_texture::gl_texture(gl_context& gl, const gl_buffer& buffer, texture_format format, size_t size,
                       size_t offset) :
    gl_texture(::shogle::gl_texture::bind_to_buffer(gl, buffer, format, size, offset).value()) {}

namespace {

auto allocate_textures(gl_context& gl, const gldefs::GLenum* texes, u32 count,
                       const gl_texture::allocate_args& args) -> gl_texture::n_err_return {
  NTF_ASSERT(texes != nullptr && count > 0);
  NTF_ASSERT(args.levels < gl_texture::MAX_MIPMAP_LEVEL);

  gldefs::GLenum err = 0;
  const auto allocate1d = [&](u32 width) -> u32 {
    NTF_ASSERT(width);
    constexpr auto type = gl_texture::TEX_TYPE_1D;
    u32 i = 0;
    for (; i < count; ++i) {
      GL_ASSERT(glBindTexture(type, texes[i]));
      err = GL_RET_ERR(glTexStorage1D(type, args.levels, args.format, width));
      if (err) {
        return i;
      }
    }
    return i;
  };
  const auto allocate2d = [&](gldefs::GLenum type, u32 width, u32 height,
                              gl_texture::multisample_opt ms) -> u32 {
    NTF_ASSERT(width);
    NTF_ASSERT(height);
    u32 i = 0;
    if (!ms) {
      for (; i < count; ++i) {
        GL_ASSERT(glBindTexture(type, texes[i]));
        err = GL_RET_ERR(glTexStorage2D(type, args.levels, args.format, width, height));
        if (err) {
          return i;
        }
      }
    } else {
      for (; i < count; ++i) {
        GL_ASSERT(glBindTexture(type, texes[i]));
        err = GL_RET_ERR(
          glTexStorage2DMultisample(type, args.levels, args.format, width, height, ms - 1));
        if (err) {
          return i;
        }
      }
    }
    return i;
  };
  const auto allocate3d = [&](gldefs::GLenum type, u32 width, u32 height, u32 depth,
                              gl_texture::multisample_opt ms) -> u32 {
    NTF_ASSERT(width);
    NTF_ASSERT(height);
    NTF_ASSERT(depth);
    u32 i = 0;
    if (ms) {
      for (; i < count; ++i) {
        GL_ASSERT(glBindTexture(type, texes[i]));
        err = GL_RET_ERR(glTexStorage3D(type, args.levels, args.format, width, height, depth));
        if (err) {
          return i;
        }
      }
    } else {
      for (; i < count; ++i) {
        GL_ASSERT(glBindTexture(type, texes[i]));
        err = GL_RET_ERR(
          glTexStorage3DMultisample(type, args.levels, args.format, width, height, depth, ms - 1));
        if (err) {
          return i;
        }
      }
    }
    return i;
  };

  u32 allocated = 0;
  constexpr auto no_ms = gl_texture::MULTISAMPLE_NONE;
  switch (args.type) {
    case gl_texture::TEX_TYPE_1D: {
      allocated = allocate1d(args.extent.width);
    } break;
    case gl_texture::TEX_TYPE_1D_ARRAY: {
      NTF_ASSERT(args.layers);
      allocated = allocate2d(args.type, args.extent.width, args.layers, no_ms);
    } break;
    case gl_texture::TEX_TYPE_CUBEMAP: {
      NTF_ASSERT(args.extent.width == args.extent.height);
      allocated = allocate2d(args.type, args.extent.width, args.extent.height, no_ms);
    } break;
    case gl_texture::TEX_TYPE_2D: {
      allocated = allocate2d(args.type, args.extent.width, args.extent.height, no_ms);
    } break;
    case gl_texture::TEX_TYPE_2D_ARRAY: {
      NTF_ASSERT(args.layers);
      allocated = allocate3d(args.type, args.extent.width, args.extent.height, args.layers, no_ms);
    } break;
    case gl_texture::TEX_TYPE_2D_MULTISAMPLE: {
      NTF_ASSERT(args.multisampling);
      allocated = allocate2d(args.type, args.extent.width, args.extent.height, args.multisampling);
    } break;
    case gl_texture::TEX_TYPE_2D_MULTISAMPLE_ARRAY: {
      NTF_ASSERT(args.multisampling);
      NTF_ASSERT(args.layers);
      allocated = allocate3d(args.type, args.extent.width, args.extent.height, args.layers,
                             args.multisampling);
    } break;
    case gl_texture::TEX_TYPE_3D: {
      allocated =
        allocate3d(args.type, args.extent.width, args.extent.height, args.extent.depth, no_ms);
    } break;
    default: {
      NTF_ASSERT(false, "Unknown texture type {}", args.type);
      NTF_UNREACHABLE();
    };
  }

  GL_ASSERT(glBindTexture(args.type, GL_DEFAULT_BINDING));
  return {allocated, err};
}

#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
std::string_view tex_format_string(gl_texture::texture_format format) {
#define STR(enum_)                     \
  case gl_texture::TEX_FORMAT_##enum_: \
    return #enum_

  switch (format) {
    default:
      NTF_UNREACHABLE();
  }

#undef STR
}

std::string_view tex_sampler_string(gldefs::GLenum sampler) {
#define STR(enum_)                      \
  case gl_texture::SAMPLER_MIN_##enum_: \
    return #enum_

  switch (sampler) {
    STR(NEAREST);
    STR(LINEAR);
    STR(NEAREST_MP_NEAREST);
    STR(LINEAR_MP_NEAREST);
    STR(NEAREST_MP_LINEAR);
    STR(LINEAR_MP_LINEAR);
    default:
      NTF_UNREACHABLE();
  }

#undef STR
}

std::string_view tex_type_string(gl_texture::texture_type type) {
  switch (type) {
    case gl_texture::TEX_TYPE_1D:
      [[fallthrough]];
    case gl_texture::TEX_TYPE_1D_ARRAY:
      return "TEX_1D";
    case gl_texture::TEX_TYPE_2D_MULTISAMPLE:
      [[fallthrough]];
    case gl_texture::TEX_TYPE_2D_MULTISAMPLE_ARRAY:
      [[fallthrough]];
    case gl_texture::TEX_TYPE_2D_ARRAY:
      [[fallthrough]];
    case gl_texture::TEX_TYPE_2D:
      return "TEX_2D";
    case gl_texture::TEX_TYPE_3D:
      return "TEX_3D";
    case gl_texture::TEX_TYPE_CUBEMAP:
      return "TEX_CUBEMAP";
    case gl_texture::TEX_TYPE_BUFFER:
      return "TEX_BUFFER";
    default:
      NTF_UNREACHABLE();
  }
}

void log_allocation([[maybe_unused]] gl_context& gl, gldefs::GLhandle tex,
                    const gl_texture::allocate_args& args) {
  NTF_ASSERT(args.multisampling <= gl_texture::MULTISAMPLE_FIXED);
  static constexpr auto ms_str = std::to_array<std::string_view>({"NONE", "NOT_FIXED", "FIXED"});

  OPENGL_ALLOC_LOG(
    "Texture allocated ({}) [type: {}, format: {}, ext: {}x{}x{}, lvl: {}, lyr: {}, ms: {}]", tex,
    tex_type_string(args.type), tex_format_string(args.format), args.extent.width,
    args.extent.height, args.extent.depth, args.levels, args.layers, ms_str[args.multisampling]);
}

void log_binding([[maybe_unused]] gl_context& gl, gldefs::GLhandle tex, const gl_buffer& buffer,
                 gl_texture::texture_format format, size_t size, size_t offset) {
  OPENGL_ALLOC_LOG("Texture bound to buffer ({}) [buff: {}, format: {}, size: {}, offset: {}]",
                   tex, buffer.id(), tex_format_string(format), size, offset);
}

void log_upload([[maybe_unused]] gl_context& gl, const gl_texture& tex,
                const gl_texture::image_data& image, const extent3d& offset, u32 lyr, u32 lvl) {
  const auto pixel_type_string = [](gl_texture::pixel_data_type type) -> std::string_view {
#define STR(enum_)                \
  case gl_texture::PIXEL_##enum_: \
    return #enum_
    switch (type) {
      STR(TYPE_I8);
      STR(TYPE_U8);
      STR(TYPE_I16);
      STR(TYPE_U16);
      STR(TYPE_I32);
      STR(TYPE_U32);
      STR(TYPE_F32);
      STR(TYPE_F16);
      STR(TYPE_U32D24S8);
      default:
        NTF_UNREACHABLE();
    }
#undef STR
  };
  const auto pixel_format_string = [](gl_texture::pixel_format format) -> std::string_view {
#define STR(enum_)                       \
  case gl_texture::PIXEL_FORMAT_##enum_: \
    return #enum_
    switch (format) {
      STR(R_NORM);
      STR(RG_NORM);
      STR(RGB_NORM);
      STR(BGR_NORM);
      STR(RGBA_NORM);
      STR(BGRA_NORM);
      STR(DEPTH);
      STR(DEPTH_STENCIL);
      default:
        NTF_UNREACHABLE();
    }
#undef STR
  };

  const auto [w, h, d] = tex.extent();
  const auto [ow, oh, od] = offset;
  OPENGL_ACTION_LOG("Texture upload ({}) [type: {}, extent: {}x{}x{}, ptr: {}, sz: {}, align: {}, "
                    "pixel_type: {}, pixel_format: {}, off: {}x{}x{}, lyr: {}, lvl: {}]",
                    tex.id(), tex_type_string(tex.type()), w, h, d, fmt::ptr(image.data),
                    image.size, (u32)image.alignment, pixel_type_string(image.datatype),
                    pixel_format_string(image.format), ow, oh, od, lyr, lvl);
}

void log_destroy([[maybe_unused]] gl_context& gl, const gl_texture& tex) {
  if (tex.type() == gl_texture::TEX_TYPE_BUFFER) {
    OPENGL_ALLOC_LOG("Texture unbound from buffer ({}) [format: {}, size: {}, offset: {}]",
                     tex.id(), tex_format_string(tex.format()), tex.buffer_size(),
                     tex.buffer_offset());
  } else {
    const auto [w, h, d] = tex.extent();
    OPENGL_ALLOC_LOG(
      "Texture deallocated ({}) [type: {}, format: {}, ext: {}x{}x{}, lvl: {}, lyr: {}]", tex.id(),
      tex_type_string(tex.type()), tex_format_string(tex.format()), w, h, d, tex.levels(),
      tex.layers());
  }
}
#endif

} // namespace

auto gl_texture::_allocate_span(gl_context& gl, span<gldefs::GLenum> texes,
                                const allocate_args& args) -> n_err_return {
  NTF_ASSERT(!texes.empty());
  GL_ASSERT(glGenTextures(texes.size(), texes.data()));
  const auto allocated = allocate_textures(gl, texes.data(), texes.size(), args);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  for (u32 i = 0; i < allocated.first; ++i) {
    log_allocation(gl, texes[i], args);
  }
#endif
  if (allocated.first < texes.size()) {
    GL_ASSERT(glDeleteTextures(texes.size() - allocated.first, texes.data() + allocated.first));
  }
  return allocated;
}

gl_expect<gl_texture> gl_texture::allocate1d(gl_context& gl, texture_format format, u32 extent,
                                             u32 levels, u32 layers) {
  gldefs::GLenum tex;
  GL_ASSERT(glGenTextures(1, &tex));
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<u32>::extent_clamp3d(extent),
    .type = layers > 1 ? TEX_TYPE_1D_ARRAY : TEX_TYPE_1D,
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = std::max(layers, 1u),
    .multisampling = MULTISAMPLE_NONE,
  };
  const auto [allocated, err] = allocate_textures(gl, &tex, 1, args);
  NTF_UNUSED(allocated);
  if (err) {
    NTF_ASSERT(allocated == 0);
    GL_ASSERT(glDeleteTextures(1, &tex));
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(allocated == 1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  log_allocation(gl, tex, args);
#endif
  return {ntf::in_place, create_t{}, tex, args};
}

gl_expect<gl_texture> gl_texture::allocate2d(gl_context& gl, texture_format format,
                                             const extent2d& extent, u32 levels, u32 layers,
                                             multisample_opt multisampling) {
  gldefs::GLenum tex;
  GL_ASSERT(glGenTextures(1, &tex));
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<extent2d>::extent_clamp3d(extent),
    .type = multisampling ? (layers > 1 ? TEX_TYPE_2D_MULTISAMPLE_ARRAY : TEX_TYPE_2D_MULTISAMPLE)
                          : (layers > 1 ? TEX_TYPE_2D_ARRAY : TEX_TYPE_2D),
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = std::max(layers, 1u),
    .multisampling = multisampling,
  };
  const auto [allocated, err] = allocate_textures(gl, &tex, 1, args);
  NTF_UNUSED(allocated);
  if (err) {
    NTF_ASSERT(allocated == 0);
    GL_ASSERT(glDeleteTextures(1, &tex));
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(allocated == 1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  log_allocation(gl, tex, args);
#endif
  return {ntf::in_place, create_t{}, tex, args};
}

gl_expect<gl_texture> gl_texture::allocate_cubemap(gl_context& gl, texture_format format,
                                                   u32 extent, u32 levels) {
  gldefs::GLenum tex;
  GL_ASSERT(glGenTextures(1, &tex));
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<extent2d>::extent_clamp3d(extent2d{extent, extent}),
    .type = TEX_TYPE_CUBEMAP,
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = 1u,
    .multisampling = MULTISAMPLE_NONE,
  };
  const auto [allocated, err] = allocate_textures(gl, &tex, 1, args);
  NTF_UNUSED(allocated);
  if (err) {
    NTF_ASSERT(allocated == 0);
    GL_ASSERT(glDeleteTextures(1, &tex));
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(allocated == 1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  log_allocation(gl, tex, args);
#endif
  return {ntf::in_place, create_t{}, tex, args};
}

gl_expect<gl_texture> gl_texture::allocate3d(gl_context& gl, texture_format format,
                                             const extent3d& extent, u32 levels) {
  gldefs::GLenum tex;
  GL_ASSERT(glGenTextures(1, &tex));
  const allocate_args args{
    .extent = ::shogle::meta::extent_traits<extent3d>::extent_clamp3d(extent),
    .type = TEX_TYPE_3D,
    .format = format,
    .levels = std::min(levels, MAX_MIPMAP_LEVEL),
    .layers = 1u,
    .multisampling = MULTISAMPLE_NONE,
  };
  const auto [allocated, err] = allocate_textures(gl, &tex, 1, args);
  NTF_UNUSED(allocated);
  if (err) {
    NTF_ASSERT(allocated == 0);
    GL_ASSERT(glDeleteTextures(1, &tex));
    return {ntf::unexpect, err};
  }
  NTF_ASSERT(allocated == 1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  log_allocation(gl, tex, args);
#endif
  return {ntf::in_place, create_t{}, tex, args};
}

gl_expect<gl_texture> gl_texture::bind_to_buffer(gl_context& gl, const gl_buffer& buffer,
                                                 texture_format format, size_t size,
                                                 size_t offset) {
  gldefs::GLenum tex;
  GL_ASSERT(glGenTextures(1, &tex));
  GL_ASSERT(glBindTexture(TEX_TYPE_BUFFER, tex));
  auto err = GL_RET_ERR(
    glTexBufferRange(TEX_TYPE_BUFFER, format, buffer.id(), (GLintptr)offset, (GLsizeiptr)size));
  GL_ASSERT(glBindTexture(TEX_TYPE_BUFFER, GL_DEFAULT_BINDING));
  if (err) {
    GL_ASSERT(glDeleteTextures(1, &tex));
    return {ntf::unexpect, err};
  }
  return {ntf::in_place, create_t{}, tex, format, size, offset};
}

void gl_texture::deallocate(gl_context& gl, gl_texture& tex) {
  auto id = tex._id;
  if (id == GL_NULL_HANDLE) {
    return;
  }
  GL_ASSERT(glDeleteTextures(1, &id));
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  log_destroy(gl, tex);
#endif
  tex._id = GL_NULL_HANDLE;
}

void gl_texture::deallocate_n(gl_context& gl, gl_texture* texes, u32 tex_count) {
  if (!texes) {
    return;
  }
  for (u32 i = 0; i < tex_count; ++i) {
    auto id = texes[i]._id;
    if (id != GL_NULL_HANDLE) {
      GL_ASSERT(glDeleteTextures(1, &id));
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
      log_destroy(gl, texes[i]);
#endif
      texes[i]._id = GL_NULL_HANDLE;
    }
  }
}

void gl_texture::deallocate_n(gl_context& gl, span<gl_texture> texes) {
  for (auto& tex : texes) {
    auto id = tex._id;
    if (id != GL_NULL_HANDLE) {
      GL_ASSERT(glDeleteTextures(1, &id));
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
      log_destroy(gl, tex);
#endif
      tex._id = GL_NULL_HANDLE;
    }
  }
}

gl_expect<void> gl_texture::upload_image(gl_context& gl, const image_data& image,
                                         const extent3d& offset, u32 layer, u32 level) {
  if (type() == gl_texture::TEX_TYPE_BUFFER) {
    return {ntf::unexpect, GL_INVALID_VALUE};
  }

  const auto [count, err] = do_upload_images(gl, id(), type(), &image, 1, offset, layer, level);
  if (err) {
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
    OPENGL_ERR_LOG("Texture upload failed ({}) [type: {}, ptr: {}, err: {}]", id(),
                   tex_type_string(type()), fmt::ptr(&image), ::shogle::gl_error_string(err));
#endif
    return {ntf::unexpect, err};
  }
  NTF_UNUSED(count);
  NTF_ASSERT(count == 1);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  log_upload(gl, *this, image, offset, layer, level);
#endif
  return {};
}

namespace {

auto do_upload_image_layers(gl_context& gl, gldefs::GLhandle tex, gl_texture::texture_type type,
                            const gl_texture::image_data* images, u32 image_count,
                            const extent3d& offset, u32 level) -> gl_texture::n_err_return {
  if (type == gl_texture::TEX_TYPE_BUFFER) {
    return {0, GL_INVALID_VALUE};
  }
  NTF_ASSERT(images != nullptr && image_count);

  gldefs::GLenum err = 0;
  const auto upload1d = [&](u32 xoff, u32 width) {
    u32 i = 0;
    for (; i < image_count; ++i) {
      const auto& image = images[i];
      err = GL_RET_ERR(glTexSubImage1D(type, level, xoff, width ? width : image.extent.width,
                                       image.format, image.datatype, image.data));
      if (err) {
        return i;
      }
    }
    return i;
  };

  GL_ASSERT(glBindTexture(type, tex));
  u32 uploaded = 0;
  switch (type) {
    case gl_texture::TEX_TYPE_1D: {
      uploaded = upload1d(offset.width, 0);
    } break;
  }
  GL_ASSERT(glBindTexture(type, GL_DEFAULT_BINDING));
  return {uploaded, err};
}

} // namespace

auto gl_texture::upload_image_layers(gl_context& gl, const image_data* layers, u32 layer_count,
                                     const extent3d& offset, u32 level) -> n_err_return {
  if (!layers || !layer_count) {
    return {0, GL_INVALID_VALUE};
  }
  layer_count = std::max(layer_count, _layers);
  const auto [count, err] =
    do_upload_image_layers(gl, id(), type(), layers, layer_count, offset, level);
  NTF_UNUSED(err);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  if (err) {
    OPENGL_ERR_LOG(
      "Texture layer upload failure ({}) [type: {}, ptr: {}, uploaded: {}/{}, err: {}]", id(),
      tex_type_string(type()), count, fmt::ptr(layers), layer_count,
      ::shogle::gl_error_string(err));
  }
  for (u32 i = 0; i < count; ++i) {
    log_upload(gl, *this, layers[i], offset, i, level);
  }
#endif
  return {count, err};
}

auto gl_texture::upload_image_layers(gl_context& gl, span<const image_data> layers,
                                     const extent3d& offset, u32 level) -> n_err_return {
  if (layers.empty()) {
    return {0, GL_INVALID_VALUE};
  }
  u32 layer_count = std::max((u32)layers.size(), _layers);
  const auto [count, err] =
    do_upload_image_layers(gl, id(), type(), layers.data(), layer_count, offset, level);
  NTF_UNUSED(err);
#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
  OPENGL_ERR_LOG("Texture layer upload failure ({}) [type: {}, ptr:{}, uploaded: {}/{}, err: {}]",
                 id(), tex_type_string(type()), count, fmt::ptr(layers.data()), layer_count,
                 ::shogle::gl_error_string(err));
  for (u32 i = 0; i < count; ++i) {
    log_upload(gl, *this, layers[i], offset, i, level);
  }
#endif
  return {count, err};
}

void gl_texture::generate_mipmaps(gl_context& gl) {
  if (!_levels) {
    return;
  }
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  GL_ASSERT(glBindTexture(type(), id()));
  GL_ASSERT(glGenerateMipmap(type()));
  GL_ASSERT(glBindTexture(type(), GL_DEFAULT_BINDING));
  // TODO: Add logger here
}

gl_texture& gl_texture::set_swizzle(gl_context& gl, swizzle_target target, swizzle_mask mask) {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  GL_ASSERT(glBindTexture(_type, _id));
  GL_ASSERT(glTexParameteri(_type, target, mask));
  GL_ASSERT(glBindTexture(_type, GL_DEFAULT_BINDING));
  // TODO: Add logger here
  return *this;
}

gl_texture& gl_texture::set_sampler_min(gl_context& gl, texture_min_sampler sampler) {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  GL_ASSERT(glBindTexture(_type, _id));
  GL_ASSERT(glTexParameteri(_type, 0x2801, sampler)); // GL_TEXTURE_MIN_FILTER
  GL_ASSERT(glBindTexture(_type, GL_DEFAULT_BINDING));
  // TODO: Add logger here
  return *this;
}

gl_texture& gl_texture::set_sampler_mag(gl_context& gl, texture_mag_sampler sampler) {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  GL_ASSERT(glBindTexture(_type, _id));
  GL_ASSERT(glTexParameteri(_type, 0x2800, sampler)); // GL_TEXTURE_MAG_FILTER
  GL_ASSERT(glBindTexture(_type, GL_DEFAULT_BINDING));
  // TODO: Add logger here
  return *this;
}

gl_texture& gl_texture::set_wrap(gl_context& gl, wrap_direction dir, texture_wrap wrap) {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  GL_ASSERT(glBindTexture(_type, _id));
  GL_ASSERT(glTexParameteri(_type, dir, wrap));
  GL_ASSERT(glBindTexture(_type, GL_DEFAULT_BINDING));
  // TODO: Add logger here
  return *this;
}

gldefs::GLhandle gl_texture::id() const {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  return _id;
}

extent3d gl_texture::extent() const {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  NTF_ASSERT(_type != TEX_TYPE_BUFFER, "Can't get extent from a buffer texture");
  return _extent;
}

auto gl_texture::type() const -> texture_type {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  return _type;
}

auto gl_texture::format() const -> texture_format {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  return _format;
}

u32 gl_texture::layers() const {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  NTF_ASSERT(_type != TEX_TYPE_BUFFER, "Can't get layers from a buffer texture");
  return _layers;
}

u32 gl_texture::levels() const {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  NTF_ASSERT(_type != TEX_TYPE_BUFFER, "Can't get levels from a buffer texture");
  return _levels;
}

size_t gl_texture::buffer_size() const {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  NTF_ASSERT(_type == TEX_TYPE_BUFFER, "Can't get a buffer size from non buffer texture");
  return (static_cast<size_t>(_extent.height) << 32) | static_cast<size_t>(_extent.width);
}

size_t gl_texture::buffer_offset() const {
  NTF_ASSERT(!invalidated(), "gl_texture use after free");
  NTF_ASSERT(_type == TEX_TYPE_BUFFER, "Can't get a buffer offset from non buffer texture");
  return (static_cast<size_t>(_levels) << 32) | static_cast<size_t>(_layers);
}

bool gl_texture::invalidated() const {
  return _id != GL_NULL_HANDLE;
}

} // namespace shogle
