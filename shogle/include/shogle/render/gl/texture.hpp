#pragma once

#include <shogle/render/res/shader.hpp>
#include <shogle/fs/res/texture.hpp>

namespace ntf::render {

class Texture {
public:
  using data_t = fs::texture_data;

protected:
  Texture(size_t w, size_t h, GLenum format, GLenum dim); // Empty texture

public: // Resources can't be copied
  Texture(const Texture::data_t* data);
  ~Texture();

  Texture(Texture&&) noexcept;
  Texture& operator=(Texture&&) noexcept;

  Texture(const Texture&) = delete;
  Texture& operator=(Texture&) = delete;

public:
  void set_filter(GLint filter);

public:
  float aspect(void) const { return (float)_w/(float)_h;}
  size_t width() const { return _w; }
  size_t height() const { return _h; }
  GLuint id(void) const { return _tex; }

public:
  bool inverted{false};

protected:
  size_t _w, _h;
  GLuint _tex;
  GLenum _dim;
};

} // namespace ntf::render
