#pragma once

#include <shogle/core/types.hpp>
#include <shogle/core/singleton.hpp>

#include <shogle/res/assets.hpp>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#define GL_DEFAULT_FRAMEBUFFER 0

namespace ntf::render {

class gl : public Singleton<gl> {
public:
  struct shader {
    using loader_t = res::shader_loader;

    shader(loader_t loader);
    ~shader();

    shader(shader&&) noexcept;
    shader(const shader&) = delete;
    shader& operator=(shader&&) noexcept;
    shader& operator=(const shader&) = delete;

    GLuint prog;
  };

  struct texture {
    using loader_t = res::texture_loader;

    texture(loader_t loader);
    texture(size_t w, size_t h, res::texture_format format, res::texture_filter filter);
    ~texture();

    texture(texture&&) noexcept;
    texture(const texture&) = delete;
    texture& operator=(texture&&) noexcept;
    texture& operator=(const texture&) = delete;

    GLuint id;

    GLenum glformat;
    GLenum gltype;
    GLint glfilter;

    res::texture_format format;
    res::texture_type type;
    res::texture_filter filter;

    size_t width, height;
  };

  struct mesh {
    using loader_t = res::model_loader::mesh;

    struct material {
      texture tex;
      std::string uniform_name;
      size_t material_num;
    };

    mesh(loader_t loader);
    ~mesh();

    mesh(mesh&&) noexcept;
    mesh(const mesh&) = delete;
    mesh& operator=(mesh&&) noexcept;
    mesh& operator=(const mesh&) = delete;

    GLuint vao; 
    GLuint vbo;
    GLuint ebo;
    size_t indices;
    std::vector<material> materials;
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
    texture tex;
  };

public:
  static void init(GLADloadproc proc);

public:
  static void set_tex_filter(texture& tex, res::texture_filter filter);

  static void draw_mesh(const mesh& mesh);
  static void draw_quad(const texture& texture, bool inverted = false);

public:
  static inline void framebuffer_bind(const framebuffer* fbo) {
    if (fbo) {
      glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_FRAMEBUFFER);
    }
  }
  static inline void texture_bind(const texture& tex, size_t number = 0) {
    glBindTexture(tex.gltype, tex.id);
    glActiveTexture(GL_TEXTURE0+number);
  }

  static inline void clear_viewport(vec4 col = {vec3{0.2f}, 1.0f}, bool clear_depth = true) {
    GLbitfield mask = GL_COLOR_BUFFER_BIT;
    if (clear_depth) {
      mask |= GL_DEPTH_BUFFER_BIT;
    }
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(mask);
  }

  static inline void depth_test(bool flag = true) {
    if (flag) {
      glEnable(GL_DEPTH_TEST);
    } else {
      glDisable(GL_DEPTH_TEST);
    }
  }

  static inline void blend(bool flag = true) {
    if (flag) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
    } else {
      glDisable(GL_BLEND);
    }
  }

  static inline void set_viewport(size_t w, size_t h) {
    glViewport(0, 0, w, h);
  }

  static inline void shader_enable(shader& shader) {
    glUseProgram(shader.prog);
  }
  static inline void shader_unif(shader& shader, const char* name, long unsigned int val) {
    glUniform1i(glGetUniformLocation(shader.prog, name), val);
  }
  static inline void shader_unif(shader& shader, const char* name, int val) {
    glUniform1i(glGetUniformLocation(shader.prog, name), val);
  }
  static inline void shader_unif(shader& shader, const char* name, float val) {
    glUniform1f(glGetUniformLocation(shader.prog, name), val);
  }
  static inline void shader_unif(shader& shader, const char* name, vec2 val) {
    glUniform2fv(glGetUniformLocation(shader.prog, name), 1, glm::value_ptr(val));
  }
  static inline void shader_unif(shader& shader, const char* name, vec3 val) {
    glUniform3fv(glGetUniformLocation(shader.prog, name), 1, glm::value_ptr(val));
  }
  static inline void shader_unif(shader& shader, const char* name, vec4 val) {
    glUniform4fv(glGetUniformLocation(shader.prog, name), 1, glm::value_ptr(val));
  }
  static inline void shader_unif(shader& shader, const char* name, mat4 val) {
    glUniformMatrix4fv(glGetUniformLocation(shader.prog, name), 1, GL_FALSE, glm::value_ptr(val));
  }
};

}

