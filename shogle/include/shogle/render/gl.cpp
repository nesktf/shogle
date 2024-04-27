#include "gl.hpp"

#include <shogle/core/error.hpp>

#include <fmt/format.h>

#define DEFAULT_FILTER GL_NEAREST
#define DEFAULT_WRAP GL_REPEAT

namespace ntf::render {

void initialize(GLADloadproc proc) {
  if (!gladLoadGLLoader(proc)) {
    throw ntf::error("[GL Renderer] Failed to load GLAD");
  }
}

void draw_meshes(const std::vector<mesh>& meshes) {

}

void draw_sprite(const texture& sprite_tex) {

}

// Todo: this thing
void set_texture_filter(texture&, texture_filter) {}

// texture
texture::texture(fs::texture_loader loader) {
  GLenum format;
  switch (loader.channels) {
    case 1: {
      format = GL_RED;
      break;
    }
    case 4: {
      format = GL_RGBA;
      break;
    }
    default: {
      format = GL_RGB;
      break;
    }
  }

  switch (loader.t_dim) {
    default: {
      dim = GL_TEXTURE_2D;
      break;
    }
  }

  glGenTextures(1, &id);
  glBindTexture(dim, id);
  glTexImage2D(dim, 0, format, loader.width, loader.height, 0, format, GL_UNSIGNED_BYTE, loader.pixels);
  glGenerateMipmap(dim);
  glTexParameteri(dim, GL_TEXTURE_MIN_FILTER, DEFAULT_FILTER);
  glTexParameteri(dim, GL_TEXTURE_MAG_FILTER, DEFAULT_FILTER);
  glTexParameteri(dim, GL_TEXTURE_WRAP_T, DEFAULT_WRAP);
  glTexParameteri(dim, GL_TEXTURE_WRAP_S, DEFAULT_WRAP);
  glBindTexture(dim, 0);
}

texture::~texture() {
  if (!id) return;
  glDeleteTextures(1, &id);
}

texture::texture(texture&& t) noexcept :
  id(t.id), dim(t.dim), filter(t.filter),
  w(t.w), h(t.h) { t.id = 0; }

texture& texture::operator=(texture&& t) noexcept {
  id = t.id; dim = t.dim; filter = t.filter;
  w = t.w; h = t.h;

  t.id = 0;

  return *this;
}

// framebuffer
framebuffer::framebuffer(size_t w, size_t h) {
  auto dim = GL_TEXTURE_2D;

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &cbo);
  glBindTexture(dim, cbo);
  glTexImage2D(dim, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(dim, 0);
  glTexParameteri(dim, GL_TEXTURE_MIN_FILTER, DEFAULT_FILTER);
  glTexParameteri(dim, GL_TEXTURE_MAG_FILTER, DEFAULT_FILTER);
  glTexParameteri(dim, GL_TEXTURE_WRAP_T, DEFAULT_WRAP);
  glTexParameteri(dim, GL_TEXTURE_WRAP_S, DEFAULT_WRAP);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cbo, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw ntf::error("[GL Renderer] Incomplete framebuffer");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

framebuffer::~framebuffer() {
  // TODO this thing
}

framebuffer::framebuffer(framebuffer&& f) noexcept :
  fbo(f.fbo), rbo(f.rbo), cbo(f.cbo) { f.fbo = 0; f.rbo = 0; f.cbo = 0; }

framebuffer& framebuffer::operator=(framebuffer&& f) noexcept {
  fbo = f.fbo; rbo = f.rbo; cbo = f.cbo;

  f.fbo = 0; f.rbo = 0; f.cbo = 0;

  return *this;
}

// mesh
mesh::mesh(fs::mesh_loader loader) {

}

mesh::~mesh() {

}

mesh::mesh(mesh&& m) noexcept :
  vao(m.vao), vbo(m.vbo), ebo(m.ebo),
  indices(m.indices), material(std::move(m.material)) { m.vao = 0; m.vbo = 0; m.ebo = 0; }

mesh& mesh::operator=(mesh&& m) noexcept {
  vao = m.vao; vbo = m.vbo; ebo = m.ebo;
  indices = m.indices; material = std::move(m.material);

  m.vao = 0; m.vbo = 0; m.ebo = 0;

  return *this;
}

// shader
shader::shader(fs::shader_loader loader) {
  int succ;
  char log[512];

  const char* vert_src = loader.vert_src.c_str();
  const char* frag_src = loader.frag_src.c_str();


  auto vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vert_src, nullptr);
  glCompileShader(vert);
  glGetShaderiv(vert, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert, 512, nullptr, log);
  }

}

shader::~shader() {

}

shader::shader(shader&& s) noexcept :
  prog(s.prog) { s.prog = 0; }

shader& shader::operator=(shader&& s) noexcept {
  prog = s.prog;

  s.prog = 0;

  return *this;
}


} // namespace ntf::render
