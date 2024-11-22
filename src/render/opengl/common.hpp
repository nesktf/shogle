#pragma once

#include "../render.hpp"

#include <glad/glad.h>

#define SHOGLE_GL_CHECK_ERROR() ::ntf::gl_check_error(__FILE__, __LINE__)

namespace ntf {

class gl_context;

template<std::size_t faces>
struct gl_texture_traits; // yes please give me a texture with UINT_MAX faces

template<>
struct gl_texture_traits<6u> {
  using data_type = std::array<uint8_t*, 6u>;
  using dim_type = std::size_t;
  static constexpr GLint gltype = GL_TEXTURE_CUBE_MAP; 
};

template<>
struct gl_texture_traits<1u> {
  using data_type = uint8_t*;
  using dim_type = ivec2;
  static constexpr GLint gltype = GL_TEXTURE_2D;
};

template<typename... T>
struct stride_sum {
  static constexpr std::size_t value = 0;
};

template<typename T, typename... U >
struct stride_sum<T, U...> {
  static constexpr std::size_t value = T::stride + stride_sum<U...>::value;
};

using gl_shader_uniform = GLint;

static constexpr bool gl_validate_uniform(gl_shader_uniform unif) {
  return unif != -1;
}

static inline GLenum gl_check_error(const char* file, int line) {
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
    ntf::logger::error("[SHOGLE][ntf::render] GL error ({}) @ \"{}\":{} -> {}",
                       err, file, line, err_str);
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

static constexpr GLint enumtogl(shader_category type) {
  switch (type) {
    case shader_category::vertex:
      return GL_VERTEX_SHADER;
    case shader_category::fragment:
      return GL_FRAGMENT_SHADER;
    case shader_category::geometry:
      return GL_GEOMETRY_SHADER;
    case shader_category::none:
      return 0;
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

} // namespace ntf
