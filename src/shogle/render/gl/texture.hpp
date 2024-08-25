#pragma once

#include <shogle/render/gl.hpp>

namespace ntf {

namespace impl {

template<size_t faces>
struct gl_tex_helper; // yes please give me a texture with INT_MAX faces

template<>
struct gl_tex_helper<SHOGLE_CUBEMAP_FACES> {
  using data_type = std::array<uint8_t*, SHOGLE_CUBEMAP_FACES>;
  using dim_type = size_t;
  static constexpr GLint gltype = GL_TEXTURE_CUBE_MAP; 
};

template<>
struct gl_tex_helper<1u> {
  using data_type = uint8_t*;
  using dim_type = ivec2;
  static constexpr GLint gltype = GL_TEXTURE_2D;
};

} // namespace impl

template<size_t faces>
class gl::texture {
public:
  using renderer = gl;
  using data_type = typename impl::gl_tex_helper<faces>::data_type;
  using dim_type = typename impl::gl_tex_helper<faces>::dim_type;

  static constexpr size_t face_count = faces;

  struct loader {
    texture operator()(data_type data, dim_type dim, tex_format format) {
      return texture{data, dim, format};
    }

    texture operator()(data_type data, dim_type dim, tex_format format, tex_filter filter, tex_wrap wrap) {
      texture tex{data, dim, format};
      tex.set_wrap(wrap);
      tex.set_filter(filter);
      return tex;
    }
  };

public:
  texture() = default;

  texture(GLuint id, dim_type dim) :
    _id(id), _dim(dim) {}

  texture(data_type data, dim_type dim, tex_format format);

public:
  texture& set_filter(tex_filter filter);
  texture& set_wrap(tex_wrap wrap);

  void bind_sampler(size_t sampler) const;
  void unload();

public:
  GLuint& id() { return _id; } // Not const
  dim_type dim() const { return _dim; }
  bool valid() const { return _id != 0; }

  operator bool() const { return valid(); }

private:
  static constexpr GLint gltype = impl::gl_tex_helper<faces>::gltype;

private:
  GLuint _id{0};
  dim_type _dim{};

public:
  NTF_DECLARE_MOVE_ONLY(texture);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_TEXTURE_INL
#include <shogle/render/gl/texture.inl>
#endif
