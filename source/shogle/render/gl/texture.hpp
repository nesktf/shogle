#pragma once

#include <shogle/render/gl/render.hpp>

#include <array>

#define DEFAULT_FILTER GL_NEAREST
#define DEFAULT_WRAP_2D GL_REPEAT
#define DEFAULT_WRAP_CUBEMAP GL_CLAMP_TO_EDGE
#define CUBEMAP_FACES 6

namespace ntf::shogle::gl {

using cubemap_pixels = std::array<unsigned char*, CUBEMAP_FACES>;

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
  // tex2d
  texture(vec2sz sz, format format, unsigned char* pixels);
  // cubemap
  texture(vec2sz sz, format format, cubemap_pixels pixels);
  // empty texture
  texture(vec2sz sz, type type, format format);

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
  GLint _filter {DEFAULT_FILTER};
  vec2sz _size;
};


} // namespace ntf::shogle::gl
