#pragma once

#include "./common.hpp"
#include "./texture.hpp"

namespace ntf {

class gl_framebuffer {
public:
  using context_type = gl_context;
  using texture_type = gl_texture<1u>;

public:
  gl_framebuffer() = default;

  gl_framebuffer(std::size_t w, std::size_t h, gl_tex_params params = {});
  gl_framebuffer(ivec2 sz, gl_tex_params params = {});

public:
  gl_framebuffer& load(std::size_t w, std::size_t h, gl_tex_params params = {}) &;
  gl_framebuffer&& load(std::size_t w, std::size_t h, gl_tex_params params = {}) &&;
  gl_framebuffer& load(ivec2 sz, gl_tex_params params = {}) &; 
  gl_framebuffer&& load(ivec2 sz, gl_tex_params params = {}) &&; 

  void unload();

  template<framebuffer_func F>
  gl_framebuffer& bind(std::size_t vp_w, std::size_t vp_h, F&& func) &;

  template<framebuffer_func F>
  gl_framebuffer&& bind(std::size_t vp_w, std::size_t vp_h, F&& func) &&;

  template<framebuffer_func F>
  gl_framebuffer& bind(ivec2 vp_sz, F&& func) &;

  template<framebuffer_func F>
  gl_framebuffer&& bind(ivec2 vp_sz, F&& func) &&;

public:
  texture_type& tex() & { return _texture; }
  const texture_type& tex() const& { return _texture; }

  GLuint id() const { return _fbo; }
  GLuint rbo() const { return _rbo; }
  ivec2 size() const { return _dim; }
  bool valid() const { return _fbo != 0 && _fbo != 0 && _texture.valid(); }

  explicit operator bool() const { return valid(); }

private:
  GLuint _fbo{0}, _rbo{0};
  texture_type _texture{};
  ivec2 _dim{0,0};

private:
  void _load(std::size_t w, std::size_t h, gl_tex_params params);
  void _reset();

  template<framebuffer_func F>
  void _bind(std::size_t vp_w, std::size_t vp_h, F&& func);

public:
  NTF_DECLARE_MOVE_ONLY(gl_framebuffer);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_OPENGL_FRAMEBUFFER_INL
#include "./framebuffer.inl"
#endif
