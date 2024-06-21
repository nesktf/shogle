#pragma once

#include <shogle/render/texture.hpp>

namespace ntf::shogle {

class framebuffer {
public:
  struct raii_bind {
    raii_bind(framebuffer& fb, vec2sz viewport);
    ~raii_bind();
    framebuffer& _fb;
    vec2sz _viewport;
  };

public:
  framebuffer(size_t w, size_t h);
  framebuffer(vec2sz sz);

public:
  framebuffer& bind();
  framebuffer& unbind(vec2sz viewport);
  raii_bind scoped_bind(vec2sz viewport);

  texture2d& tex() { return _texture; }

public:
  GLuint id() const { return _fbo; }
  vec2sz size() const { return _size; }

public:
  ~framebuffer();
  framebuffer(framebuffer&&) noexcept;
  framebuffer(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) = delete;

private:
  texture2d _texture;
  GLuint _fbo, _rbo;
  vec2sz _size;
};

} // namespace ntf::shogle
