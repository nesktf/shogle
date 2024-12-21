#pragma once

#include "./opengl.hpp"

namespace ntf {
class gl_framebuffer;

class gl_texture {
private:
  gl_texture(gl_context& ctx) :
    _ctx(ctx) {}

private:
  void load(r_texture_type type, r_texture_format format,
            r_texture_sampler sampler, r_texture_address addressing,
            const uint8* const* texels, uint32 mipmaps, uint32 count, uvec3 dim);
  void unload();

private:
  GLuint _allocate(GLenum gltype, GLenum glformat, uint32 count, uint32 mipmaps, uvec3 dim);
  void _upload(GLenum gltype, GLenum glformat,
               const uint8* texels, uint32 mipmap, uint32 index, uvec3 offset);
  void _set_sampler(GLenum gltype, GLenum glsamplermin, GLenum gladdressmag);
  void _set_addressing(GLenum gltype, GLenum gladdress);

public:
  void data(r_texture_format format, const uint8* texels, uint32 index, uvec3 offset);
  void sampler(r_texture_sampler sampler);
  void addressing(r_texture_address address);

public:
  uint32 dim_x() const { return _dim.x; }
  uvec2 dim_xy() const { return uvec2{_dim.x, _dim.y}; }
  uvec3 dim_xyz() const { return _dim; }

  r_texture_type type() const { return _type; }
  r_texture_sampler sampler() const { return _sampler; }
  r_texture_address addressing() const { return _addressing; }
  r_texture_format format() const { return _format; }

  uint32 count() const { return _count; }
  uint32 mipmaps() const { return _mipmaps; }

  bool is_array() const { return _count > 1 && _type != r_texture_type::cubemap; }
  bool has_mipmaps() const { return _mipmaps > 0; }

private:
  bool complete() const { return _id != 0; }

private:
  gl_context& _ctx;

  GLuint _id{0};
  uvec3 _dim{0, 0, 0};
  r_texture_type _type{r_texture_type::none};
  r_texture_address _addressing{r_texture_address::none};
  r_texture_sampler _sampler{r_texture_sampler::none};
  r_texture_format _format{r_texture_format::none};
  uint32 _count{0};
  uint32 _mipmaps{0};

private:
  friend class gl_context;
  friend class gl_framebuffer;
};

constexpr inline GLenum gl_texture_type_cast(r_texture_type type, bool array) {
  switch (type) {
    case r_texture_type::texture1d: return array ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
    case r_texture_type::texture2d: return array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case r_texture_type::texture3d: return GL_TEXTURE_3D;
    case r_texture_type::cubemap:   return GL_TEXTURE_CUBE_MAP;

    case r_texture_type::none:      return 0;
  };
  return 0;
}

constexpr inline GLenum gl_texture_format_cast(r_texture_format format) {
  switch (format) {
    case r_texture_format::mono:    return GL_RED;
    case r_texture_format::rgb:     return GL_RGB;
    case r_texture_format::rgba:    return GL_RGBA;

    case r_texture_format::none:    return 0;
  };
  return 0;
}

constexpr inline GLenum gl_texture_sampler_cast(r_texture_sampler sampler, bool mipmaps) {
  switch (sampler) {
    case r_texture_sampler::nearest:  return mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case r_texture_sampler::linear:   return mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    // TODO: change this?
    // case r_texture_sampler::linear: {
    //   if (mipmaps) {
    //     if (lin_lvl) {
    //       return lin_lvls ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
    //     } else {
    //       return lin_lvl ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
    //     }
    //   } else {
    //     return GL_LINEAR;
    //   }
    //   break;
    // }

    case r_texture_sampler::none:     return 0;
  }
  return 0;
}

constexpr inline GLenum gl_texture_address_cast(r_texture_address address) {
  switch (address) {
    case r_texture_address::clamp_border:         return GL_CLAMP_TO_BORDER;
    case r_texture_address::repeat:               return GL_REPEAT;
    case r_texture_address::repeat_mirrored:      return GL_MIRRORED_REPEAT;
    case r_texture_address::clamp_edge:           return GL_CLAMP_TO_EDGE;
    case r_texture_address::clamp_edge_mirrored:  return GL_MIRROR_CLAMP_TO_EDGE;

    case r_texture_address::none:                 return 0;
  };
  return 0;
}

} // namespace ntf
