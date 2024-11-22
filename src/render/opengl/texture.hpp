#pragma once

#include "./common.hpp"

namespace ntf {

struct gl_tex_params {
  tex_filter filter = tex_filter::nearest;
  tex_wrap wrap = tex_wrap::repeat;
  bool gen_mipmaps = true;
};

template<std::size_t faces>
class gl_texture {
public:
  using context_type = gl_context;
  using data_type = typename gl_texture_traits<faces>::data_type;
  using dim_type = typename gl_texture_traits<faces>::dim_type;

  static constexpr std::size_t face_count = faces;
  static constexpr GLint gltype = gl_texture_traits<faces>::gltype;

public:
  gl_texture() = default;

  gl_texture(data_type data, dim_type dim, tex_format format, gl_tex_params params = {});

public:
  gl_texture& load(data_type data, dim_type dim, tex_format format, gl_tex_params params = {}) &;
  gl_texture&& load(data_type data, dim_type dim, tex_format format, gl_tex_params params = {}) &&;

  gl_texture& filter(tex_filter filter) &;
  gl_texture& wrap(tex_wrap wrap) &;

  void unload();

public:
  GLuint id() const { return _id; }
  dim_type dim() const { return _dim; }
  bool valid() const { return _id != 0; }

  explicit operator bool() const { return valid(); }

private:
  GLuint _id{0};
  dim_type _dim{};

private:
  void _load(data_type data, dim_type dim, tex_format format, gl_tex_params params);
  void _set_filter(tex_filter filter, bool bind);
  void _set_wrap(tex_wrap wrap, bool bind);
  void _reset();

public:
  NTF_DECLARE_MOVE_ONLY(gl_texture);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_OPENGL_TEXTURE_INL
#include "./texture.inl"
#endif
