#pragma once

#include "./opengl.hpp"

namespace ntf {

class gl_texture {
protected:
  gl_texture(gl_context& ctx) :
    _ctx(&ctx) {}

  void load(r_texture_type type, r_texture_format format,
            r_texture_sampler sampler, r_texture_address addressing,
            const uint8** texels, uint32 mipmaps, uint32 count, uvec3 dim);
  void unload();

protected:
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

protected:
  bool complete() const { return _id != 0; }

public:
  gl_context* _ctx;

  GLuint _id{0};
  uvec3 _dim{0, 0, 0};
  r_texture_type _type{r_texture_type::none};
  r_texture_address _addressing{r_texture_address::none};
  r_texture_sampler _sampler{r_texture_sampler::none};
  r_texture_format _format{r_texture_format::none};
  uint32 _count{0};
  uint32 _mipmaps{0};

public:
  NTF_DISABLE_MOVE_COPY(gl_texture); // Managed by the context

private:
  friend class gl_context;
};

GLenum gl_texture_type_cast(r_texture_type type, bool array);
GLenum gl_texture_format_cast(r_texture_format format);
GLenum gl_texture_sampler_cast(r_texture_sampler sampler, bool mipmaps);
GLenum gl_texture_address_cast(r_texture_address address);

} // namespace ntf
