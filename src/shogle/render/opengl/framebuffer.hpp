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

  gl_framebuffer(GLuint fbo, GLuint rbo, GLuint texture, std::size_t w, std::size_t h) :
    _fbo(fbo), _rbo(rbo), _texture(texture, ivec2{w, h}) {};

  gl_framebuffer(ivec2 sz) 
    { load(sz); }

  gl_framebuffer(std::size_t w, std::size_t h)
    { load(w, h); }

public:
  void load(ivec2 sz);
  void load(std::size_t w, std::size_t h);

  void unload();

  template<typename Renderer>
  void bind (ivec2 vp_sz, Renderer&& renderer) {
    assert(valid() && "Invalid gl_framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, _dim.x, _dim.y);
    renderer();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, vp_sz.x, vp_sz.y);
  }

  template<typename Renderer>
  void bind(std::size_t viewport_w, std::size_t viewport_h, Renderer&& renderer) {
    bind(ivec2{static_cast<int>(viewport_w), static_cast<int>(viewport_h)},
         std::forward<Renderer>(renderer));
  }

public:
  texture_type& tex() { return _texture; }
  const texture_type& tex() const { return _texture; }

  GLuint& id() { return _fbo; } // Not const
  ivec2 size() const { return _dim; }
  bool valid() const { return _fbo != 0 && _fbo != 0 && _texture.valid(); }

  explicit operator bool() const { return valid(); }

private:
  GLuint _fbo{}, _rbo{};
  texture_type _texture{};
  ivec2 _dim{};

public:
  NTF_DECLARE_MOVE_ONLY(gl_framebuffer);
};

} // namespace ntf
