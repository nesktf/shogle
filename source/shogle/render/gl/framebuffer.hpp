#pragma once

#include <shogle/render/gl/texture.hpp>

namespace ntf::shogle::gl {

class framebuffer {
public:
  struct raii_bind {
    raii_bind(framebuffer& fb, vec2sz viewport);
    ~raii_bind();
    framebuffer& _fb;
    vec2sz _viewport;
  };

public:
  framebuffer(vec2sz sz);

public:
  framebuffer& bind();
  framebuffer& unbind(vec2sz viewport);
  raii_bind scoped_bind(vec2sz viewport);

public:
  GLuint id() const { return _fbo; }
  vec2sz size() const { return _size; }
  const texture& tex() const { return _texture; }

public:
  ~framebuffer();
  framebuffer(framebuffer&&) noexcept;
  framebuffer(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) noexcept;

private:
  texture _texture;
  GLuint _fbo;
  GLuint _rbo;
  vec2sz _size;
};

} // namespace ntf::shogle::gl
