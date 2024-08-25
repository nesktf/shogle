#pragma once

#ifndef SHOGLE_GL_RAII_UNLOAD
#define SHOGLE_GL_RAII_UNLOAD 1
#endif

#include <shogle/render/common.hpp>

#include <glad/glad.h>

namespace ntf {

class gl {
public:
  static constexpr int VER_MAJOR = 3;
  static constexpr int VER_MINOR = 3;
  static constexpr int DEFAULT_FRAMEBUFFER = 0;

public:
  template<typename GLLoader>
  static bool init(GLLoader proc);

  static void destroy();
  static const char* name_str();

private:
  static void init_meshes();

public:
  static void start_frame();
  static void end_frame();

public:
  static void set_viewport(size_t w, size_t h);
  static void set_viewport(ivec2 sz);
  static void set_viewport(size_t x, size_t y, size_t w, size_t h);
  static void set_viewport(ivec2 pos, ivec2 sz);

  static void clear_viewport(color4 color, clear flag = clear::none);
  static void clear_viewport(color3 color, clear flag = clear::none);

  static void set_stencil_test(bool flag);
  static void set_depth_test(bool flag);
  static void set_blending(bool flag);

  static void set_depth_fun(depth_fun fun);

public:
  template<size_t faces>
  class texture;

  using texture2d = texture<1u>;
  using cubemap = texture<SHOGLE_CUBEMAP_FACES>;

  using shader_uniform = GLint;
  class shader;
  class shader_program;
  class uniform_list;
  class shader_args;

  class mesh;

  class framebuffer;

  using font_atlas = std::map<uint8_t, std::pair<GLuint, font_glyph>>;
  class font;

public:
  static void draw_quad();
  static void draw_cube();
  static void draw_text(const font_atlas& atlas, vec2 pos, float scale, std::string_view text);

public:
  static inline GLenum check_error(const char* file, int line) {
    GLenum err{};
    while ((err = glGetError()) != GL_NO_ERROR) {
      std::string err_str;
      switch (err) {
        case GL_INVALID_ENUM: { err_str = "INVALID_ENUM"; break; }
        case GL_INVALID_VALUE: { err_str = "INVALID_VALUE"; break; }
        case GL_INVALID_OPERATION: { err_str = "INVALID_OPERATION"; break; }
        case GL_STACK_OVERFLOW: { err_str = "STACK_OVERFLOW"; break; }
        case GL_STACK_UNDERFLOW: { err_str = "STACK_UNDERFLOW"; break; }
        case GL_OUT_OF_MEMORY: { err_str = "OUT_OF_MEMORY"; break; }
        case GL_INVALID_FRAMEBUFFER_OPERATION: { err_str = "INVALID_FRAMEBUFFER_OPERATION"; break; }
        default: { err_str = "UNKNOWN_ERROR"; break; }
      }
      ntf::log::error("[SHOGLE][ntf::render] GL error ({}) @ \"{}\":{} -> {}", err, file, line, err_str);
    }
    return err;
  }

  static constexpr GLint enumtogl(tex_format format) {
    switch (format) {
      case tex_format::rgb:
        return GL_RGB;
      case tex_format::mono:
        return GL_RED;
      case tex_format::rgba:
        return GL_RGBA;
    }
    return 0; // shutup gcc
  }

  static constexpr GLint enumtogl(tex_filter filter) {
    switch (filter) {
      case tex_filter::linear:
        return GL_LINEAR;
      case tex_filter::nearest:
        return GL_NEAREST;
      case tex_filter::nearest_mp_nearest:
        return GL_NEAREST_MIPMAP_NEAREST;
      case tex_filter::nearest_mp_linear:
        return GL_NEAREST_MIPMAP_LINEAR;
      case tex_filter::linear_mp_linear:
        return GL_LINEAR_MIPMAP_LINEAR;
      case tex_filter::linear_mp_nearest:
        return GL_LINEAR_MIPMAP_NEAREST;
    }
    return 0; // shutup gcc
  }

  static constexpr GLint enumtogl(tex_wrap wrap) {
    switch (wrap) {
      case tex_wrap::repeat:
        return GL_REPEAT;
      case tex_wrap::mirrored_repeat:
        return GL_MIRRORED_REPEAT;
      case tex_wrap::clamp_edge:
        return GL_CLAMP_TO_EDGE;
      case tex_wrap::clamp_border:
        return GL_CLAMP_TO_BORDER;
    }
    return 0; // shutup gcc
  }

  static constexpr GLint enumtogl(shader_type type) {
    switch (type) {
      case shader_type::vertex:
        return GL_VERTEX_SHADER;
      case shader_type::fragment:
        return GL_FRAGMENT_SHADER;
      case shader_type::geometry:
        return GL_GEOMETRY_SHADER;
    }
    return 0; // shutup gcc
  }

  static constexpr size_t enumtosz(uniform_type type) {
    switch (type) {
      case uniform_type::scalar:
        return sizeof(float);
      case uniform_type::iscalar:
        return sizeof(int);
      case uniform_type::vec2:
        return sizeof(vec2);
      case uniform_type::vec3:
        return sizeof(vec3);
      case uniform_type::vec4:
        return sizeof(vec4);
      case uniform_type::mat3:
        return sizeof(mat3);
      case uniform_type::mat4:
        return sizeof(mat4);
    }
    return 0; // shutup gcc
  }

  static constexpr GLint enumtogl(mesh_buffer type) {
    switch (type) {
      case mesh_buffer::static_draw:
        return GL_STATIC_DRAW;
      case mesh_buffer::dynamic_draw:
        return GL_DYNAMIC_DRAW;
      case mesh_buffer::stream_draw:
        return GL_STREAM_DRAW;
    }
    return 0; // shutup gcc
  }

  static constexpr GLint enumtogl(mesh_primitive primitive) {
    switch (primitive) {
      case mesh_primitive::triangles:
        return GL_TRIANGLES;
      case mesh_primitive::triangle_strip:
        return GL_TRIANGLE_STRIP;
      case mesh_primitive::triangle_fan:
        return GL_TRIANGLE_FAN;
      case mesh_primitive::lines:
        return GL_LINES;
      case mesh_primitive::line_strip:
        return GL_LINE_STRIP;
      case mesh_primitive::line_loop:
        return GL_LINE_LOOP;
      case mesh_primitive::points:
        return GL_POINTS;
    }
    return 0; // shutup gcc
  }
};

template<typename GLLoader>
bool gl::init(GLLoader proc) {
  if (!gladLoadGLLoader((GLADloadproc)proc)) {
    return false;
  }

  init_meshes();
  return true;
}

} // namespace ntf

#define SHOGLE_GL_CHECK_ERROR() ::ntf::gl_renderer::check_error(__FILE__, __LINE__)
