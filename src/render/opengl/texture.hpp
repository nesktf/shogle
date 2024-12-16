#pragma once

#include "./common.hpp"

namespace ntf {

class gl_texture {
protected:
  gl_texture(gl_context& ctx) :
    _ctx(&ctx) {}

  bool load(const uint8** texels, uint32 count, uint32 mipmaps, uvec3 dim,
            r_texture_type type, r_texture_format format,
            r_texture_sampler sampler, r_texture_address address);
  void unload();

public:
  void data(const uint8* texels, uint32 index, uvec3 offset, r_texture_format format);
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

uint32 gl_texture_type(r_texture_type type, uint32 count);
uint32 gl_texture_format(r_texture_format format);
uint32 gl_texture_sampler(r_texture_sampler sampler, bool mipmaps);
uint32 gl_texture_address(r_texture_address address);

} // namespace ntf
