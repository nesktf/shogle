#pragma once

#include "./common.hpp"

namespace ntf {

template<std::size_t faces>
class gl_texture {
public:
  using context_type = gl_context;
  using data_type = typename gl_texture_traits<faces>::data_type;
  using dim_type = typename gl_texture_traits<faces>::dim_type;

  static constexpr std::size_t face_count = faces;

  struct loader {
    gl_texture operator()(data_type data, dim_type dim, tex_format format) {
      return gl_texture{data, dim, format};
    }

    gl_texture operator()(data_type data, dim_type dim, tex_format format,
                       tex_filter filter, tex_wrap wrap) {
      gl_texture tex{data, dim, format};
      tex.set_wrap(wrap);
      tex.set_filter(filter);
      return tex;
    }
  };

public:
  gl_texture() = default;

  gl_texture(GLuint id, dim_type dim) :
    _id(id), _dim(dim) {}

  gl_texture(data_type data, dim_type dim, tex_format format) 
    { load(std::move(data), dim, format); }

public:
  void load(data_type data, dim_type dim, tex_format format);
  void unload();

  gl_texture& set_filter(tex_filter filter);
  gl_texture& set_wrap(tex_wrap wrap);

  void bind_sampler(std::size_t sampler) const;

public:
  GLuint& id() { return _id; } // Not const
  dim_type dim() const { return _dim; }
  bool valid() const { return _id != 0; }

  explicit operator bool() const { return valid(); }

private:
  static constexpr GLint gltype = gl_texture_traits<faces>::gltype;

private:
  GLuint _id{0};
  dim_type _dim{};

public:
  NTF_DECLARE_MOVE_ONLY(gl_texture);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_OPENGL_TEXTURE_INL
#include "./texture.inl"
#endif
