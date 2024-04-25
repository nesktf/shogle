#pragma once

#include <shogle/render/shader.hpp>

namespace ntf::render {

// Texture::data_t
class TextureData {
public: // Resource data can be copied but i don't think is a good idea
  TextureData(std::string path);
  ~TextureData();

  TextureData(TextureData&&) noexcept;
  TextureData& operator=(TextureData&&) noexcept;

  TextureData(const TextureData&) = delete; TextureData& operator=(const TextureData&) = delete;
public:
  int width, height, channels;
  GLenum dim {GL_TEXTURE_2D};
  unsigned char* tex_data;
};

// Texture
class Texture {
public:
  using data_t = TextureData;

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
