#pragma once

#include "./texture.hpp"

namespace ntf {

class gl_framebuffer {
private:
  gl_framebuffer(gl_context& ctx) :
    _ctx(ctx) {}

private:
  void load(uint32 x, uint32 y, uint32 w, uint32 h,
            r_texture_sampler sampler, r_texture_address addressing);
  void unload();

public:
  void viewport(uint32 x, uint32 y, uint32 w, uint32 h);
  void viewport(uint32 w, uint32 h);
  void viewport(uvec2 pos, uvec2 size);
  void viewport(uvec2 size);

  void clear_color(float32 r, float32 g, float32 b, float32 a);
  void clear_color(float32 r, float32 g, float32 b);
  void clear_color(color4 color);
  void clear_color(color3 color);

  void clear_flags(r_clear clear);

public:
  gl_texture& tex() { return _tex.get(); }
  const gl_texture& tex() const { return _tex.get(); }
  r_texture_view texview() const {return _tex; }

  uvec4 viewport() const { return _viewport; }
  uvec2 viewport_size() const { return uvec2{_viewport.z, _viewport.w}; }
  uvec2 viewport_pos() const { return uvec2{_viewport.x, _viewport.y}; }

  r_clear clear_flags() const { return _clear; }
  color4 clear_color() const { return _clear_color; }

private:
  bool complete() const { return _fbo != 0; }

private:
  gl_context& _ctx;
  r_handle<gl_context, gl_texture, r_texture_view> _tex{};
  // gl_texture _tex;

  GLuint _fbo{0}, _rbo{0};
  uvec4 _viewport{0, 0, 0, 0};
  r_clear _clear{r_clear::none};
  color4 _clear_color{.3f, .3f, .3f, 1.f};

private:
  friend class gl_context;
};

} // namespace ntf
