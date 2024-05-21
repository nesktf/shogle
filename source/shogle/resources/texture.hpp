#pragma once

#include <shogle/render/gl/texture.hpp>

namespace ntf::shogle::resources {

struct spritesheet_data;

struct texture2d_data {
private:
  friend spritesheet_data;
  texture2d_data() = default;

public:
  texture2d_data(std::string _path);

public:
  ~texture2d_data();
  texture2d_data(texture2d_data&&) noexcept;
  texture2d_data(const texture2d_data&) = delete;
  texture2d_data& operator=(texture2d_data&&) noexcept;
  texture2d_data& operator=(const texture2d_data&) = delete;

public:
  std::string path;
  int width {1}, height {1}, channels {0};
  gl::texture::format format;
  unsigned char* pixels {nullptr};
};

struct cubemap_data {
public:
  cubemap_data(std::string _path);

public:
  ~cubemap_data();
  cubemap_data(cubemap_data&&) noexcept;
  cubemap_data(const cubemap_data&) = delete;
  cubemap_data& operator=(cubemap_data&&) noexcept;
  cubemap_data& operator=(const cubemap_data&) = delete;

public:
  std::string path;
  int width {1}, height {1}, channels {0};
  gl::texture::format format;
  unsigned char* pixels[CUBEMAP_FACES] {nullptr};
};

template<typename T>
class texture {
public:
  using data_t = T;

public:
  texture(std::string path) :
    texture(data_t{std::move(path)}) {}

  texture(data_t data);

public:
  GLuint tex_id() const { return _texture.id(); }
  vec2sz size() const { return _texture.size(); }

  gl::texture& tex() { return _texture; }
  void set_filter(gl::texture::filter filter) { _texture.set_filter(filter); }

  std::string path() const { return _path; }

private:
  std::string _path;
  gl::texture _texture;
};

template<>
texture<texture2d_data>::texture(data_t data);

template<>
texture<cubemap_data>::texture(data_t data);


using texture2d = texture<texture2d_data>;
using cubemap = texture<cubemap_data>;

} // namespace ntf::shogle::resources
