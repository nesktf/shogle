#pragma once

#include <shogle/render/gl/render.hpp>

#define CUBEMAP_FACES 6

namespace ntf::shogle::gl {

class texture {
public:
  enum class type {
    tex2d = 0,
    cubemap,
  };

  enum class format {
    mono = 0,
    rgb,
    rgba,
  };

  enum class filter {
    nearest = 0,
    linear,
  };

public:
  texture(vec2sz sz, type type, format format, unsigned char** pixels = NULL);

public:
  texture& set_filter(filter filter);

  void bind(size_t sampler) const;

public:
  GLuint id() const { return _id; }
  GLenum type() const { return _type; }
  vec2sz size() const { return _size; }

public:
  ~texture();
  texture(texture&&) noexcept;
  texture(const texture&) = delete;
  texture& operator=(texture&&) noexcept;
  texture& operator=(const texture&) = delete;

private:
  GLuint _id;
  GLenum _format;
  GLenum _type;
  GLint _filter;
  vec2sz _size;
};


} // namespace ntf::shogle::gl
