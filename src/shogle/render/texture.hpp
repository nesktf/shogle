#pragma once

#include <shogle/render/render.hpp>

#include <type_traits>
#include <array>

#define CUBEMAP_FACES 6

namespace ntf::shogle {

enum class tex_format {
  mono = 0,
  rgb,
  rgba,
};

enum class tex_filter {
  nearest = 0,
  linear,
  nearest_mp_nearest,
  nearest_mp_linear,
  linear_mp_linear,
  linear_mp_nearest,
};

enum class tex_wrap {
  repeat = 0,
  mirrored_repeat,
  clamp_edge,
  clamp_border,
};

using __tex2d_data = unsigned char*;
using __cubemap_data = std::array<unsigned char*, CUBEMAP_FACES>;

template<typename T>
class __texture {
public:
  __texture(vec2sz dim, tex_format format, tex_filter filter, tex_wrap wrap, T data);
  __texture(vec2sz dim, tex_format format, tex_filter filter, tex_wrap wrap);

public:
  inline __texture& set_filter(tex_filter filter);
  inline __texture& set_wrap(tex_wrap wrap);

  inline void bind_sampler(size_t sampler);

public:
  GLuint id() const { return _id; }
  vec2sz dim() const { return _dim; }

public:
  ~__texture();
  __texture(__texture&&) noexcept;
  __texture(const __texture&) = delete;
  __texture& operator=(__texture&&) noexcept;
  __texture& operator=(const __texture&) = delete;

private:
  static const constexpr GLint _type = 
    std::is_same_v<T, __cubemap_data> ? GL_TEXTURE_CUBE_MAP : 
    std::is_same_v<T, __tex2d_data> ? GL_TEXTURE_2D :
    GL_NONE;

private:
  vec2sz _dim;
  GLint _format, _filter, _wrap;
  GLuint _id;
};

using texture2d = __texture<__tex2d_data>;
using cubemap = __texture<__cubemap_data>;

} // namespace ntf::shogle

#ifndef SHOGLE_TEXTURE_INL_HPP
#include <shogle/render/texture.inl.hpp>
#endif
