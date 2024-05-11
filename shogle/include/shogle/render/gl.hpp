#pragma once

#include <shogle/core/types.hpp>
#include <shogle/core/singleton.hpp>

#include <shogle/res/asset_loaders.hpp>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

#define GL_DEFAULT_FRAMEBUFFER 0

namespace ntf::render {

class gl : public Singleton<gl> {
public:
  struct shader {
    using loader_t = res::shader_loader;

    shader(loader_t loader);
    shader(const char* vert_src, const char* frag_src);

    GLuint prog;
  };

  struct texture {
    using loader_t = res::texture_loader;

    texture(loader_t loader);
    texture(size_t w, size_t h, res::texture_format tex_format, res::texture_filter tex_filter);

    GLuint id;
    GLenum format;
    GLenum type;
    GLint filter;
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

    GLuint vao; 
    GLuint vbo;
    GLuint ebo;
    size_t indices;
    std::vector<material> materials;
  };

  struct framebuffer {
    framebuffer(size_t w, size_t h);

    GLuint fbo;
    GLuint rbo;
    texture tex;
  };

public:
  static void init(GLADloadproc proc);
  static void destroy(void);

public:
  static void destroy_shader(shader& shad);
  static void destroy_texture(texture& tex);
  static void destroy_mesh(mesh& mesh);
  static void destroy_framebuffer(framebuffer& fbo);

public:
  static void set_tex_filter(texture& tex, res::texture_filter filter);

  static void draw_mesh(const mesh& mesh);
  static void draw_quad_2d(bool inverted = false);
  static void draw_quad_3d(bool inverted = false);
  static void draw_cube(void);
  static void draw_cubemap(void);

public:
  static inline void framebuffer_bind(const framebuffer* fbo) {
    if (fbo) {
      glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
    } else {
      glBindFramebuffer(GL_FRAMEBUFFER, GL_DEFAULT_FRAMEBUFFER);
    }
  }
  static inline void texture_bind(const texture& tex, size_t number = 0) {
    glBindTexture(tex.type, tex.id);
    glActiveTexture(GL_TEXTURE0+number);
  }

  static inline void clear_viewport(vec4 col = {vec3{0.2f}, 1.0f}, bool clear_depth = true) {
    GLbitfield mask = GL_COLOR_BUFFER_BIT;
    if (clear_depth) {
      mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (clear_depth) {
      mask |= GL_STENCIL_BUFFER_BIT;
    }
    glClearColor(col.r, col.g, col.b, col.a);
    glClear(mask);
  }

  static inline void stencil_test(bool flag = true) {
    if (flag) {
      glEnable(GL_STENCIL_TEST);
    } else {
      glDisable(GL_STENCIL_TEST);
    }
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

