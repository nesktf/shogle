#pragma once

#include <shogle/render/gl/texture.hpp>

namespace ntf {

class gl_renderer::framebuffer {
public:
  using renderer_type = gl_renderer;
  using texture_type = gl_renderer::texture2d;

public:
  framebuffer() = default;

  framebuffer(GLuint fbo, GLuint rbo, GLuint texture, size_t w, size_t h) :
    _fbo(fbo), _rbo(rbo), _texture(texture, ivec2{w, h}) {};
  framebuffer(ivec2 sz) :
    framebuffer(sz.x, sz.y) {}

  framebuffer(size_t w, size_t h);

public:
  void unload();

  template<typename Renderer>
  void bind (ivec2 vp_sz, Renderer&& renderer);

  template<typename Renderer>
  void bind(size_t viewport_w, size_t viewport_h, Renderer&& renderer);

public:
  const texture_type& tex() const { return _texture; }

  GLuint& id() { return _fbo; } // Not const
  ivec2 size() const { return _dim; }
  bool valid() const { return _fbo != 0 && _fbo != 0 && _texture.valid(); }

  operator bool() const { return valid(); }

private:
  GLuint _fbo{}, _rbo{};
  texture_type _texture{};
  ivec2 _dim{};

public:
  NTF_DECLARE_MOVE_ONLY(framebuffer);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_FRAMEBUFFER_INL
#include <shogle/render/gl/framebuffer.inl>
#endif
