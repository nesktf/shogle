#pragma once

#include <shogle/core/types.hpp>
#include <shogle/fs/resources.hpp>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace ntf::render {

enum class texture_filter {
  nearest = 0,
  linear
};

struct texture {
  texture(fs::texture_loader loader);
  ~texture();

  texture(texture&&) noexcept;
  texture(const texture&) = delete;
  texture& operator=(texture&&) noexcept;
  texture& operator=(const texture&) = delete;

  GLuint id;
  GLenum dim;
  GLint filter;
  size_t w, h;
};

struct framebuffer {
  framebuffer(size_t w, size_t h);
  ~framebuffer();

  framebuffer(framebuffer&&) noexcept;
  framebuffer(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) noexcept;
  framebuffer& operator=(const framebuffer&) = delete;

  GLuint fbo;
  GLuint rbo;
  GLuint cbo;
};

struct mesh {
  mesh(fs::mesh_loader loader);
  ~mesh();

  mesh(mesh&&) noexcept;
  mesh(const mesh&) = delete;
  mesh& operator=(mesh&&) noexcept;
  mesh& operator=(const mesh&) = delete;

  GLuint vao; 
  GLuint vbo;
  GLuint ebo;
  size_t indices;
  std::vector<std::pair<texture, std::string>> material;
};

struct shader {
  shader(fs::shader_loader loader);
  ~shader();

  shader(shader&&) noexcept;
  shader(const shader&) = delete;
  shader& operator=(shader&&) noexcept;
  shader& operator=(const shader&) = delete;

  GLuint prog;
};

void initialize(GLADloadproc proc);

void set_texture_filter(texture& tex, texture_filter filter);

void draw_meshes(const std::vector<mesh>& meshes);
void draw_sprite(const texture& sprite_tex);

inline void clear_viewport(vec4 col = {vec3{0.2f}, 1.0f}, bool clear_depth = true) {
  GLbitfield mask = GL_COLOR_BUFFER_BIT;
  if (clear_depth) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }
  glClearColor(col.r, col.g, col.b, col.a);
  glClear(mask);
}

inline void set_viewport(size_t w, size_t h) {
  glViewport(0, 0, w, h);
}

inline void use_shader(shader& shader) {
  glUseProgram(shader.prog);
}
inline void shader_unif(shader& shader, const char* name, int val) {
  glUniform1i(glGetUniformLocation(shader.prog, name), val);
}
inline void shader_unif(shader& shader, const char* name, float val) {
  glUniform1f(glGetUniformLocation(shader.prog, name), val);
}
inline void shader_unif(shader& shader, const char* name, vec2 val) {
  glUniform2fv(glGetUniformLocation(shader.prog, name), 1, glm::value_ptr(val));
}
inline void shader_unif(shader& shader, const char* name, vec3 val) {
  glUniform3fv(glGetUniformLocation(shader.prog, name), 1, glm::value_ptr(val));
}
inline void shader_unif(shader& shader, const char* name, vec4 val) {
  glUniform4fv(glGetUniformLocation(shader.prog, name), 1, glm::value_ptr(val));
}
inline void shader_unif(shader& shader, const char* name, mat4 val) {
  glUniformMatrix4fv(glGetUniformLocation(shader.prog, name), 1, GL_FALSE, glm::value_ptr(val));
}

}

