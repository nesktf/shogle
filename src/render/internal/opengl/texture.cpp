#include "./state.hpp"

namespace ntf {

static constexpr int32 DEFAULT_IMAGE_ALIGNMENT = static_cast<uint32>(r_image_alignment::bytes4);

GLenum gl_state::texture_type_cast(r_texture_type type, bool is_array) noexcept {
  switch (type) {
    case r_texture_type::texture1d:       return is_array ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
    case r_texture_type::texture2d:       return is_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case r_texture_type::texture3d:       return GL_TEXTURE_3D;
    case r_texture_type::cubemap:         return GL_TEXTURE_CUBE_MAP;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::r8n:       return GL_R8_SNORM;
    case r_texture_format::r8nu:      return GL_R8;
    case r_texture_format::r8u:       return GL_R8UI;
    case r_texture_format::r8i:       return GL_R8I;
    case r_texture_format::r16u:      return GL_R16UI;
    case r_texture_format::r16i:      return GL_R16I;
    case r_texture_format::r16f:      return GL_R16F;
    case r_texture_format::r32u:      return GL_R32UI;
    case r_texture_format::r32i:      return GL_R32I;
    case r_texture_format::r32f:      return GL_R32F;

    case r_texture_format::rg8n:      return GL_RG8_SNORM;
    case r_texture_format::rg8nu:     return GL_RG;
    case r_texture_format::rg8u:      return GL_RG8UI;
    case r_texture_format::rg8i:      return GL_RG8I;
    case r_texture_format::rg16u:     return GL_RG16UI;
    case r_texture_format::rg16i:     return GL_RG16I;
    case r_texture_format::rg16f:     return GL_RG16F;
    case r_texture_format::rg32u:     return GL_RG32UI;
    case r_texture_format::rg32i:     return GL_RG32I;
    case r_texture_format::rg32f:     return GL_RG32F;

    case r_texture_format::rgb8n:     return GL_RGB8_SNORM;
    case r_texture_format::rgb8nu:    return GL_RGB8;
    case r_texture_format::rgb8u:     return GL_RGB8UI;
    case r_texture_format::rgb8i:     return GL_RGB8I;
    case r_texture_format::rgb16u:    return GL_RGB16UI;
    case r_texture_format::rgb16i:    return GL_RGB16I;
    case r_texture_format::rgb16f:    return GL_RGB16F;
    case r_texture_format::rgb32u:    return GL_RGB32UI;
    case r_texture_format::rgb32i:    return GL_RGB32I;
    case r_texture_format::rgb32f:    return GL_RGB32F;

    case r_texture_format::rgba8n:    return GL_RGBA8_SNORM;
    case r_texture_format::rgba8nu:   return GL_RGBA8;
    case r_texture_format::rgba8u:    return GL_RGBA8UI;
    case r_texture_format::rgba8i:    return GL_RGBA8UI;
    case r_texture_format::rgba16u:   return GL_RGBA16UI;
    case r_texture_format::rgba16i:   return GL_RGBA16I;
    case r_texture_format::rgba16f:   return GL_RGBA16F;
    case r_texture_format::rgba32u:   return GL_RGBA32UI;
    case r_texture_format::rgba32i:   return GL_RGBA32I;
    case r_texture_format::rgba32f:   return GL_RGBA32F;

    case r_texture_format::srgb8u:    return GL_SRGB8;
    case r_texture_format::srgba8u:   return GL_SRGB8_ALPHA8;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_symbolic_cast(r_texture_format format) noexcept {

  switch (format) {
    case r_texture_format::r8n:       [[fallthrough]];
    case r_texture_format::r8nu:      [[fallthrough]];
    case r_texture_format::r8u:       [[fallthrough]];
    case r_texture_format::r8i:       [[fallthrough]];
    case r_texture_format::r16u:      [[fallthrough]];
    case r_texture_format::r16i:      [[fallthrough]];
    case r_texture_format::r16f:      [[fallthrough]];
    case r_texture_format::r32u:      [[fallthrough]];
    case r_texture_format::r32i:      [[fallthrough]];
    case r_texture_format::r32f:      return GL_RED;

    case r_texture_format::rg8n:      [[fallthrough]];
    case r_texture_format::rg8nu:     [[fallthrough]];
    case r_texture_format::rg8u:      [[fallthrough]];
    case r_texture_format::rg8i:      [[fallthrough]];
    case r_texture_format::rg16u:     [[fallthrough]];
    case r_texture_format::rg16i:     [[fallthrough]];
    case r_texture_format::rg16f:     [[fallthrough]];
    case r_texture_format::rg32u:     [[fallthrough]];
    case r_texture_format::rg32i:     [[fallthrough]];
    case r_texture_format::rg32f:     return GL_RG;

    case r_texture_format::rgb8n:     [[fallthrough]];
    case r_texture_format::rgb8nu:    [[fallthrough]];
    case r_texture_format::rgb8u:     [[fallthrough]];
    case r_texture_format::rgb8i:     [[fallthrough]];
    case r_texture_format::rgb16u:    [[fallthrough]];
    case r_texture_format::rgb16i:    [[fallthrough]];
    case r_texture_format::rgb16f:    [[fallthrough]];
    case r_texture_format::rgb32u:    [[fallthrough]];
    case r_texture_format::rgb32i:    [[fallthrough]];
    case r_texture_format::rgb32f:    return GL_RGB;

    case r_texture_format::rgba8n:    [[fallthrough]];
    case r_texture_format::rgba8nu:   [[fallthrough]];
    case r_texture_format::rgba8u:    [[fallthrough]];
    case r_texture_format::rgba8i:    [[fallthrough]];
    case r_texture_format::rgba16u:   [[fallthrough]];
    case r_texture_format::rgba16i:   [[fallthrough]];
    case r_texture_format::rgba16f:   [[fallthrough]];
    case r_texture_format::rgba32u:   [[fallthrough]];
    case r_texture_format::rgba32i:   [[fallthrough]];
    case r_texture_format::rgba32f:   return GL_RGBA;

    case r_texture_format::srgb8u:    [[fallthrough]];
    case r_texture_format::srgba8u:   return GL_SRGB;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_underlying_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::r8u:       [[fallthrough]];
    case r_texture_format::r8nu:      [[fallthrough]];
    case r_texture_format::r8n:       [[fallthrough]];
    case r_texture_format::rg8u:      [[fallthrough]];
    case r_texture_format::rg8n:      [[fallthrough]];
    case r_texture_format::rg8nu:     [[fallthrough]];
    case r_texture_format::rgb8u:     [[fallthrough]];
    case r_texture_format::rgb8n:     [[fallthrough]];
    case r_texture_format::rgb8nu:    [[fallthrough]];
    case r_texture_format::rgba8u:    [[fallthrough]];
    case r_texture_format::rgba8n:    [[fallthrough]];
    case r_texture_format::rgba8nu:   [[fallthrough]];
    case r_texture_format::srgb8u:    [[fallthrough]];
    case r_texture_format::srgba8u:   return GL_UNSIGNED_BYTE;

    case r_texture_format::r8i:       [[fallthrough]];
    case r_texture_format::rg8i:      [[fallthrough]];
    case r_texture_format::rgb8i:     [[fallthrough]];
    case r_texture_format::rgba8i:    return GL_BYTE;

    case r_texture_format::r16u:      [[fallthrough]];
    case r_texture_format::rg16u:     [[fallthrough]];
    case r_texture_format::rgb16u:    [[fallthrough]];
    case r_texture_format::rgba16u:   return GL_UNSIGNED_SHORT;

    case r_texture_format::r16i:      [[fallthrough]];
    case r_texture_format::rg16i:     [[fallthrough]];
    case r_texture_format::rgb16i:    [[fallthrough]];
    case r_texture_format::rgba16i:   return GL_SHORT;

    case r_texture_format::r16f:      [[fallthrough]];
    case r_texture_format::rg16f:     [[fallthrough]];
    case r_texture_format::rgb16f:    [[fallthrough]];
    case r_texture_format::rgba16f:   return GL_HALF_FLOAT;

    case r_texture_format::r32u:      [[fallthrough]];
    case r_texture_format::rg32u:     [[fallthrough]];
    case r_texture_format::rgb32u:    [[fallthrough]];
    case r_texture_format::rgba32u:   return GL_UNSIGNED_INT;

    case r_texture_format::r32i:      [[fallthrough]];
    case r_texture_format::rg32i:     [[fallthrough]];
    case r_texture_format::rgb32i:    [[fallthrough]];
    case r_texture_format::rgba32i:   return GL_INT;

    case r_texture_format::r32f:      [[fallthrough]];
    case r_texture_format::rg32f:     [[fallthrough]];
    case r_texture_format::rgb32f:    [[fallthrough]];
    case r_texture_format::rgba32f:   return GL_FLOAT;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_sampler_cast(r_texture_sampler sampler, bool mipmaps) noexcept {
  // TODO: Add cases for GL_LINEAR_MIPMAP_NEAREST and GL_NEAREST_MIPMAP_LINEAR?
  switch (sampler) {
    case r_texture_sampler::nearest:  return mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case r_texture_sampler::linear:   return mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_addressing_cast(r_texture_address address) noexcept {
  switch (address) {
    case r_texture_address::clamp_border:         return GL_CLAMP_TO_BORDER;
    case r_texture_address::repeat:               return GL_REPEAT;
    case r_texture_address::repeat_mirrored:      return GL_MIRRORED_REPEAT;
    case r_texture_address::clamp_edge:           return GL_CLAMP_TO_EDGE;
    case r_texture_address::clamp_edge_mirrored:  return GL_MIRROR_CLAMP_TO_EDGE;
  };

  NTF_UNREACHABLE();
}

auto gl_state::create_texture(r_texture_type type, r_texture_format format,
                              r_texture_sampler sampler, r_texture_address addressing,
                              uvec3 extent, uint32 layers, uint32 levels) -> texture_t {
  NTF_ASSERT(levels <= MAX_TEXTURE_LEVEL && levels > 0);

  const GLenum gltype = texture_type_cast(type, (layers > 1));
  const GLenum glformat = texture_format_cast(format);

  GLuint id;
  GL_CALL(glGenTextures, 1, &id);

  auto& [active_id, active_type] = _bound_texs[_active_tex];
  GL_CALL(glBindTexture, gltype, id);
  active_id = id;
  active_type = gltype;

  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      NTF_ASSERT(layers > 0);
      GL_CALL(glTexStorage2D, gltype, levels, glformat, extent.x, layers);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      GL_CALL(glTexStorage1D, gltype, levels, glformat, extent.x);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      NTF_ASSERT(extent.y > 0 && extent.y <= _tex_limits.max_dim);
      NTF_ASSERT(layers > 0);
      GL_CALL(glTexStorage3D, gltype, levels, glformat, extent.x, extent.y, layers);
      break;
    }

    case GL_TEXTURE_CUBE_MAP:
      NTF_ASSERT(extent.x == extent.y);
      [[fallthrough]];
    case GL_TEXTURE_2D: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      NTF_ASSERT(extent.y > 0 && extent.y <= _tex_limits.max_dim);
      GL_CALL(glTexStorage2D, gltype, levels, glformat, extent.x, extent.y);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim3d);
      NTF_ASSERT(extent.y > 0 && extent.y <= _tex_limits.max_dim3d);
      NTF_ASSERT(extent.z > 0 && extent.z <= _tex_limits.max_dim3d);
      GL_CALL(glTexStorage3D, gltype, levels, glformat, extent.x, extent.y, extent.z);
      break;
    };

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };

  const GLenum glsamplermin = texture_sampler_cast(sampler, (levels > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  GL_CALL(glTexParameteri, gltype, GL_TEXTURE_MAG_FILTER, glsamplermag);
  GL_CALL(glTexParameteri, gltype, GL_TEXTURE_MIN_FILTER, glsamplermin);

  const GLenum gladdress = texture_addressing_cast(addressing);
  GL_CALL(glTexParameteri, gltype, GL_TEXTURE_WRAP_S, gladdress); // U
  if (gltype != GL_TEXTURE_1D || gltype != GL_TEXTURE_1D_ARRAY) {
    GL_CALL(glTexParameteri, gltype, GL_TEXTURE_WRAP_T, gladdress); // V
    if (gltype == GL_TEXTURE_3D || gltype == GL_TEXTURE_CUBE_MAP) {
      GL_CALL(glTexParameteri, gltype, GL_TEXTURE_WRAP_R, gladdress); // W (?)
    }
  }

  texture_t tex;
  tex.id = id;
  tex.type = gltype;
  tex.format = glformat;
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
  tex.addressing = gladdress;
  tex.extent = extent;
  tex.layers = layers;
  tex.levels = levels;

  // if (data) {
  //   update_texture_data(tex, data, format, uvec3{0, 0, 0}, 0, 0, genmips);
  // }

  return tex;
}

void gl_state::destroy_texture(const texture_t& tex) noexcept {
  NTF_ASSERT(tex.id);
  auto& [id, type] = _bound_texs[_active_tex];
  if (id == tex.id) {
    GL_CALL(glBindTexture, type, NULL_BINDING);
    id = NULL_BINDING;
    type = NULL_BINDING;
  }
  GLuint texid = tex.id;
  GL_CALL(glDeleteTextures, 1, &texid);
}

void gl_state::bind_texture(GLuint id, GLenum type, uint32 index) noexcept {
  NTF_ASSERT(type);
  NTF_ASSERT(index < _bound_texs.size());
  auto& [bound, bound_type] = _bound_texs[index];
  if (bound == id) {
    return;
  }
  GL_CALL(glActiveTexture, GL_TEXTURE0+index);
  GL_CALL(glBindTexture, type, id);
  bound = id;
  bound_type = type;
}

void gl_state::update_texture_data(const texture_t& tex, const void* data, 
                                   r_texture_format format, r_image_alignment alignment,
                                   uvec3 offset, uint32 layer,
                                   uint32 level) noexcept {
  NTF_ASSERT(tex.id);
  NTF_ASSERT(data);
  NTF_ASSERT(level < tex.levels);
  // const GLenum data_format = texture_format_cast(format);
  const GLenum data_format = texture_format_symbolic_cast(format);
  const GLenum data_format_type = texture_format_underlying_cast(format);

  NTF_ASSERT(data_format != GL_SRGB, "Can't use SRGB with glTexSubImage");
  glPixelStorei(GL_UNPACK_ALIGNMENT, static_cast<int32>(alignment));

  bind_texture(tex.id, tex.type, _active_tex);
  switch (tex.type) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(layer < tex.layers);
      GL_CALL(glTexSubImage2D, tex.type, level,
                               offset.x, layer,
                               tex.extent.x, tex.layers,
                               data_format, data_format_type, data);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      GL_CALL(glTexSubImage1D, tex.type, level,
                               offset.x, tex.extent.x,
                               data_format, data_format_type, data);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(layer < tex.layers);
      GL_CALL(glTexSubImage3D, tex.type, level,
                               offset.x, offset.y, layer, 
                               tex.extent.x, tex.extent.y, tex.layers,
                               data_format, data_format_type, data);
      break;
    }

    case GL_TEXTURE_2D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      GL_CALL(glTexSubImage2D, tex.type, level,
                               offset.x, offset.y,
                               tex.extent.x, tex.extent.y,
                               data_format, data_format_type, data);
      break;
    }
    
    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(layer < 6);
      GL_CALL(glTexSubImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X+layer, level,
                               offset.x, offset.y,
                               tex.extent.x, tex.extent.y,
                               data_format, data_format_type, data);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(offset.z < tex.extent.z);
      GL_CALL(glTexSubImage3D, tex.type, level,
                               offset.x, offset.y, offset.z,
                               tex.extent.x, tex.extent.y, tex.extent.z,
                               data_format, data_format_type, data);
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, DEFAULT_IMAGE_ALIGNMENT);
}

void gl_state::update_texture_sampler(texture_t& tex, r_texture_sampler sampler) noexcept {
  NTF_ASSERT(tex.id);
  const GLenum glsamplermin = texture_sampler_cast(sampler, (tex.levels > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  if (tex.sampler[0] == glsamplermin && tex.sampler[1] == glsamplermag) {
    return;
  }

  bind_texture(tex.id, tex.type, _active_tex);
  GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_MAG_FILTER, glsamplermag);
  GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_MIN_FILTER, glsamplermin);
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
}

void gl_state::update_texture_addressing(texture_t& tex, r_texture_address addressing) noexcept {
  NTF_ASSERT(tex.id);
  const GLenum gladdress = texture_addressing_cast(addressing);
  if (tex.addressing == gladdress) {
    return;
  }

  bind_texture(tex.id, tex.type, _active_tex);
  GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_WRAP_S, gladdress); // U
  if (tex.type != GL_TEXTURE_1D || tex.type != GL_TEXTURE_1D_ARRAY) {
    GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_WRAP_T, gladdress); // V
    if (tex.type == GL_TEXTURE_3D || tex.type == GL_TEXTURE_CUBE_MAP) {
      GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_WRAP_R, gladdress); // W (?)
    }
  }
  tex.addressing = gladdress;
}

void gl_state::gen_texture_mipmaps(const texture_t& tex) {
  bind_texture(tex.id, tex.type, _active_tex);
  GL_CALL(glGenerateMipmap, tex.type);
}

} // namespace ntf
