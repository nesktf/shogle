#pragma once

#include <shogle/render/texture.hpp>

namespace ntf::render {

class Framebuffer : public Texture {
public:
  struct fbo_raii {
    fbo_raii(GLuint id) : _id(id) { 
      glBindFramebuffer(GL_FRAMEBUFFER, _id); 
    }
    ~fbo_raii() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    GLuint _id;
  };

public:
  Framebuffer(size_t width, size_t height, GLint filter = GL_RGB, GLenum dim = GL_TEXTURE_2D);

public:
  fbo_raii bind_raii(void) { return fbo_raii{_tex}; }
  void bind(void) const { glBindFramebuffer(GL_FRAMEBUFFER, _tex); }
  void unbind(void) const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

private:
  GLuint _rbo, _fbo;
};

} // namespace ntf::render
