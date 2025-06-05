#include "./context.hpp"

namespace ntf::render {

static constexpr int32 DEFAULT_IMAGE_ALIGNMENT = 4;

GLenum gl_state::texture_type_cast(texture_type type, bool is_array) noexcept {
  switch (type) {
    case texture_type::texture1d:       return is_array ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
    case texture_type::texture2d:       return is_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case texture_type::texture3d:       return GL_TEXTURE_3D;
    case texture_type::cubemap:         return GL_TEXTURE_CUBE_MAP;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_cast(image_format format) noexcept {
  switch (format) {
    case image_format::r8n:       return GL_R8_SNORM;
    case image_format::r8nu:      return GL_R8;
    case image_format::r8u:       return GL_R8UI;
    case image_format::r8i:       return GL_R8I;
    case image_format::r16u:      return GL_R16UI;
    case image_format::r16i:      return GL_R16I;
    case image_format::r16f:      return GL_R16F;
    case image_format::r32u:      return GL_R32UI;
    case image_format::r32i:      return GL_R32I;
    case image_format::r32f:      return GL_R32F;

    case image_format::rg8n:      return GL_RG8_SNORM;
    case image_format::rg8nu:     return GL_RG;
    case image_format::rg8u:      return GL_RG8UI;
    case image_format::rg8i:      return GL_RG8I;
    case image_format::rg16u:     return GL_RG16UI;
    case image_format::rg16i:     return GL_RG16I;
    case image_format::rg16f:     return GL_RG16F;
    case image_format::rg32u:     return GL_RG32UI;
    case image_format::rg32i:     return GL_RG32I;
    case image_format::rg32f:     return GL_RG32F;

    case image_format::rgb8n:     return GL_RGB8_SNORM;
    case image_format::rgb8nu:    return GL_RGB8;
    case image_format::rgb8u:     return GL_RGB8UI;
    case image_format::rgb8i:     return GL_RGB8I;
    case image_format::rgb16u:    return GL_RGB16UI;
    case image_format::rgb16i:    return GL_RGB16I;
    case image_format::rgb16f:    return GL_RGB16F;
    case image_format::rgb32u:    return GL_RGB32UI;
    case image_format::rgb32i:    return GL_RGB32I;
    case image_format::rgb32f:    return GL_RGB32F;

    case image_format::rgba8n:    return GL_RGBA8_SNORM;
    case image_format::rgba8nu:   return GL_RGBA8;
    case image_format::rgba8u:    return GL_RGBA8UI;
    case image_format::rgba8i:    return GL_RGBA8UI;
    case image_format::rgba16u:   return GL_RGBA16UI;
    case image_format::rgba16i:   return GL_RGBA16I;
    case image_format::rgba16f:   return GL_RGBA16F;
    case image_format::rgba32u:   return GL_RGBA32UI;
    case image_format::rgba32i:   return GL_RGBA32I;
    case image_format::rgba32f:   return GL_RGBA32F;

    case image_format::srgb8u:    return GL_SRGB8;
    case image_format::srgba8u:   return GL_SRGB8_ALPHA8;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_symbolic_cast(image_format format) noexcept {

  switch (format) {
    case image_format::r8n:       [[fallthrough]];
    case image_format::r8nu:      [[fallthrough]];
    case image_format::r8u:       [[fallthrough]];
    case image_format::r8i:       [[fallthrough]];
    case image_format::r16u:      [[fallthrough]];
    case image_format::r16i:      [[fallthrough]];
    case image_format::r16f:      [[fallthrough]];
    case image_format::r32u:      [[fallthrough]];
    case image_format::r32i:      [[fallthrough]];
    case image_format::r32f:      return GL_RED;

    case image_format::rg8n:      [[fallthrough]];
    case image_format::rg8nu:     [[fallthrough]];
    case image_format::rg8u:      [[fallthrough]];
    case image_format::rg8i:      [[fallthrough]];
    case image_format::rg16u:     [[fallthrough]];
    case image_format::rg16i:     [[fallthrough]];
    case image_format::rg16f:     [[fallthrough]];
    case image_format::rg32u:     [[fallthrough]];
    case image_format::rg32i:     [[fallthrough]];
    case image_format::rg32f:     return GL_RG;

    case image_format::rgb8n:     [[fallthrough]];
    case image_format::rgb8nu:    [[fallthrough]];
    case image_format::rgb8u:     [[fallthrough]];
    case image_format::rgb8i:     [[fallthrough]];
    case image_format::rgb16u:    [[fallthrough]];
    case image_format::rgb16i:    [[fallthrough]];
    case image_format::rgb16f:    [[fallthrough]];
    case image_format::rgb32u:    [[fallthrough]];
    case image_format::rgb32i:    [[fallthrough]];
    case image_format::rgb32f:    return GL_RGB;

    case image_format::rgba8n:    [[fallthrough]];
    case image_format::rgba8nu:   [[fallthrough]];
    case image_format::rgba8u:    [[fallthrough]];
    case image_format::rgba8i:    [[fallthrough]];
    case image_format::rgba16u:   [[fallthrough]];
    case image_format::rgba16i:   [[fallthrough]];
    case image_format::rgba16f:   [[fallthrough]];
    case image_format::rgba32u:   [[fallthrough]];
    case image_format::rgba32i:   [[fallthrough]];
    case image_format::rgba32f:   return GL_RGBA;

    case image_format::srgb8u:    [[fallthrough]];
    case image_format::srgba8u:   return GL_SRGB;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_underlying_cast(image_format format) noexcept {
  switch (format) {
    case image_format::r8u:       [[fallthrough]];
    case image_format::r8nu:      [[fallthrough]];
    case image_format::r8n:       [[fallthrough]];
    case image_format::rg8u:      [[fallthrough]];
    case image_format::rg8n:      [[fallthrough]];
    case image_format::rg8nu:     [[fallthrough]];
    case image_format::rgb8u:     [[fallthrough]];
    case image_format::rgb8n:     [[fallthrough]];
    case image_format::rgb8nu:    [[fallthrough]];
    case image_format::rgba8u:    [[fallthrough]];
    case image_format::rgba8n:    [[fallthrough]];
    case image_format::rgba8nu:   [[fallthrough]];
    case image_format::srgb8u:    [[fallthrough]];
    case image_format::srgba8u:   return GL_UNSIGNED_BYTE;

    case image_format::r8i:       [[fallthrough]];
    case image_format::rg8i:      [[fallthrough]];
    case image_format::rgb8i:     [[fallthrough]];
    case image_format::rgba8i:    return GL_BYTE;

    case image_format::r16u:      [[fallthrough]];
    case image_format::rg16u:     [[fallthrough]];
    case image_format::rgb16u:    [[fallthrough]];
    case image_format::rgba16u:   return GL_UNSIGNED_SHORT;

    case image_format::r16i:      [[fallthrough]];
    case image_format::rg16i:     [[fallthrough]];
    case image_format::rgb16i:    [[fallthrough]];
    case image_format::rgba16i:   return GL_SHORT;

    case image_format::r16f:      [[fallthrough]];
    case image_format::rg16f:     [[fallthrough]];
    case image_format::rgb16f:    [[fallthrough]];
    case image_format::rgba16f:   return GL_HALF_FLOAT;

    case image_format::r32u:      [[fallthrough]];
    case image_format::rg32u:     [[fallthrough]];
    case image_format::rgb32u:    [[fallthrough]];
    case image_format::rgba32u:   return GL_UNSIGNED_INT;

    case image_format::r32i:      [[fallthrough]];
    case image_format::rg32i:     [[fallthrough]];
    case image_format::rgb32i:    [[fallthrough]];
    case image_format::rgba32i:   return GL_INT;

    case image_format::r32f:      [[fallthrough]];
    case image_format::rg32f:     [[fallthrough]];
    case image_format::rgb32f:    [[fallthrough]];
    case image_format::rgba32f:   return GL_FLOAT;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_sampler_cast(texture_sampler sampler, bool mipmaps) noexcept {
  // TODO: Add cases for GL_LINEAR_MIPMAP_NEAREST and GL_NEAREST_MIPMAP_LINEAR?
  switch (sampler) {
    case texture_sampler::nearest:  return mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case texture_sampler::linear:   return mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_addressing_cast(texture_addressing address) noexcept {
  switch (address) {
    case texture_addressing::clamp_border:         return GL_CLAMP_TO_BORDER;
    case texture_addressing::repeat:               return GL_REPEAT;
    case texture_addressing::repeat_mirrored:      return GL_MIRRORED_REPEAT;
    case texture_addressing::clamp_edge:           return GL_CLAMP_TO_EDGE;
    case texture_addressing::clamp_edge_mirrored:  return GL_MIRROR_CLAMP_TO_EDGE;
  };

  NTF_UNREACHABLE();
}

ctx_tex_status gl_state::create_texture(gltex_t& tex, texture_type type, image_format format,
                                        texture_sampler sampler, texture_addressing addressing,
                                        extent3d extent, uint32 layers, uint32 levels)
{
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
  tex.id = id;
  tex.type = gltype;
  tex.format = glformat;
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
  tex.addressing = gladdress;
  tex.extent = extent;
  tex.layers = layers;
  tex.levels = levels;
  
  return CTX_TEX_STATUS_OK;
}

void gl_state::destroy_texture(gltex_t& tex) {
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

bool gl_state::texture_bind(GLuint id, GLenum type, uint32 index) {
  NTF_ASSERT(type);
  NTF_ASSERT(index < _bound_texs.size());
  auto& [bound, bound_type] = _bound_texs[index];
  if (bound == id) {
    return false;
  }
  GL_CALL(glActiveTexture, GL_TEXTURE0+index);
  GL_CALL(glBindTexture, type, id);
  bound = id;
  bound_type = type;
  return true;
}

ctx_tex_status gl_state::texture_upload(gltex_t& tex, const void* texels, image_format format,
                                        image_alignment alignment, extent3d offset,
                                        uint32 layer, uint32 level)
{
  NTF_ASSERT(tex.id);
  NTF_ASSERT(texels);
  NTF_ASSERT(level < tex.levels);
  NTF_ASSERT(alignment % 2 == 0 && alignment <= 8);
  // const GLenum data_format = texture_format_cast(format);
  const GLenum data_format = texture_format_symbolic_cast(format);
  const GLenum data_format_type = texture_format_underlying_cast(format);

  NTF_ASSERT(data_format != GL_SRGB, "Can't use SRGB with glTexSubImage");
  glPixelStorei(GL_UNPACK_ALIGNMENT, static_cast<int32>(alignment));

  texture_bind(tex.id, tex.type, _active_tex);
  switch (tex.type) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(layer < tex.layers);
      GL_CALL(glTexSubImage2D, tex.type, level,
                               offset.x, layer,
                               tex.extent.x, tex.layers,
                               data_format, data_format_type, texels);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      GL_CALL(glTexSubImage1D, tex.type, level,
                               offset.x, tex.extent.x,
                               data_format, data_format_type, texels);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(layer < tex.layers);
      GL_CALL(glTexSubImage3D, tex.type, level,
                               offset.x, offset.y, layer, 
                               tex.extent.x, tex.extent.y, tex.layers,
                               data_format, data_format_type, texels);
      break;
    }

    case GL_TEXTURE_2D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      GL_CALL(glTexSubImage2D, tex.type, level,
                               offset.x, offset.y,
                               tex.extent.x, tex.extent.y,
                               data_format, data_format_type, texels);
      break;
    }
    
    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(layer < 6);
      GL_CALL(glTexSubImage2D, GL_TEXTURE_CUBE_MAP_POSITIVE_X+layer, level,
                               offset.x, offset.y,
                               tex.extent.x, tex.extent.y,
                               data_format, data_format_type, texels);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(offset.z < tex.extent.z);
      GL_CALL(glTexSubImage3D, tex.type, level,
                               offset.x, offset.y, offset.z,
                               tex.extent.x, tex.extent.y, tex.extent.z,
                               data_format, data_format_type, texels);
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, DEFAULT_IMAGE_ALIGNMENT);
  return CTX_TEX_STATUS_OK;
}

bool gl_state::texture_set_sampler(gltex_t& tex, texture_sampler sampler) {
  NTF_ASSERT(tex.id);
  const GLenum glsamplermin = texture_sampler_cast(sampler, (tex.levels > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  if (tex.sampler[0] == glsamplermin && tex.sampler[1] == glsamplermag) {
    return false;
  }

  texture_bind(tex.id, tex.type, _active_tex);
  GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_MAG_FILTER, glsamplermag);
  GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_MIN_FILTER, glsamplermin);
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
  return true;
}

bool gl_state::texture_set_addressing(gltex_t& tex, texture_addressing addressing) {
  NTF_ASSERT(tex.id);
  const GLenum gladdress = texture_addressing_cast(addressing);
  if (tex.addressing == gladdress) {
    return false;
  }

  texture_bind(tex.id, tex.type, _active_tex);
  GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_WRAP_S, gladdress); // U
  if (tex.type != GL_TEXTURE_1D || tex.type != GL_TEXTURE_1D_ARRAY) {
    GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_WRAP_T, gladdress); // V
    if (tex.type == GL_TEXTURE_3D || tex.type == GL_TEXTURE_CUBE_MAP) {
      GL_CALL(glTexParameteri, tex.type, GL_TEXTURE_WRAP_R, gladdress); // W (?)
    }
  }
  tex.addressing = gladdress;
  return true;
}

bool gl_state::texture_gen_mipmaps(gltex_t& tex) {
  NTF_ASSERT(tex.id);
  NTF_ASSERT(tex.levels > 1);
  texture_bind(tex.id, tex.type, _active_tex);
  return GL_CHECK(glGenerateMipmap, tex.type) == GL_NO_ERROR;
}

ctx_tex_status gl_context::create_texture(ctx_tex& tex, const ctx_tex_desc& desc) {
  ctx_tex handle = _textures.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& texture = _textures.get(handle);
  auto status = _state.create_texture(texture, desc.type, desc.format, 
                                      desc.sampler, desc.addressing,
                                      desc.extent, desc.layers, desc.levels);
  if (status != CTX_TEX_STATUS_OK) {
    _textures.push(handle);
    return status;
  }
  NTF_ASSERT(texture.id);
  if (!desc.images.empty()) {
    status = update_texture(tex, {
      .images = desc.images,
      .generate_mipmaps = desc.gen_mipmaps,
    });
  }
  if (status != CTX_TEX_STATUS_OK) {
    _state.destroy_texture(texture);
    _textures.push(handle);
    return status;
  }
  tex = handle;
  return status;
}

ctx_tex_status gl_context::destroy_texture(ctx_tex tex) noexcept {
  if (!_textures.validate(tex)) {
    return CTX_TEX_STATUS_INVALID_HANDLE;
  }
  auto& texture = _textures.get(tex);
  _state.destroy_texture(texture);
  _textures.push(tex);
  return CTX_TEX_STATUS_OK;
}

ctx_tex_status gl_context::update_texture(ctx_tex tex, const ctx_tex_data& data) {
  if (!_textures.validate(tex)) {
    return CTX_TEX_STATUS_INVALID_HANDLE;
  }
  auto& texture = _textures.get(tex);
  for (const auto& image : data.images) {
    auto status = _state.texture_upload(texture, image.bitmap, image.format,
                                        image.alignment, image.offset,
                                        image.layer, image.level);
    if (status != CTX_TEX_STATUS_OK){
      return status;
    }
  }
  if (data.generate_mipmaps) {
    // TODO: check if the texture can generate mipmaps based on its size
    if (texture.levels < 2) {
      return CTX_TEX_STATUS_INVALID_LEVELS;
    }
    if (!_state.texture_gen_mipmaps(texture)) {
      return CTX_TEX_STATUS_INVALID_LEVELS;
    }
  }
  return CTX_TEX_STATUS_OK;
}

ctx_tex_status gl_context::update_texture(ctx_tex tex, const ctx_tex_opts& opts) {
  if (!_textures.validate(tex)) {
    return CTX_TEX_STATUS_INVALID_HANDLE;
  }
  auto& texture = _textures.get(tex);
  bool add_succ = _state.texture_set_addressing(texture, opts.addresing);
  bool sam_succ = _state.texture_set_sampler(texture, opts.sampler);
  if (!add_succ) {
    return CTX_TEX_STATUS_INVALID_ADDRESING;
  }
  if (!sam_succ) {
    return CTX_TEX_STATUS_INVALID_SAMPLER;
  }
  return CTX_TEX_STATUS_OK;
}

} // namespace ntf::render
