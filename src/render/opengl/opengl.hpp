#pragma once

#include "../render.hpp"

#include <glad/glad.h>

#define glCheckError() ::ntf::gl_check_error(__FILE__, __LINE__)

namespace ntf {

class gl_context;

GLenum gl_check_error(const char* file, int line);
void gl_clear_bits(r_clear clear, const color4& color);

constexpr inline GLenum gl_attrib_underlying_type_cast(r_attrib_type type) {
  switch (type) {
    case r_attrib_type::f32:   [[fallthrough]];
    case r_attrib_type::vec2:  [[fallthrough]];
    case r_attrib_type::vec3:  [[fallthrough]];
    case r_attrib_type::vec4:  [[fallthrough]];
    case r_attrib_type::mat3:  [[fallthrough]];
    case r_attrib_type::mat4:  return GL_FLOAT;

    case r_attrib_type::f64:   [[fallthrough]];
    case r_attrib_type::dvec2: [[fallthrough]];
    case r_attrib_type::dvec3: [[fallthrough]];
    case r_attrib_type::dvec4: [[fallthrough]];
    case r_attrib_type::dmat3: [[fallthrough]];
    case r_attrib_type::dmat4: return GL_DOUBLE;

    case r_attrib_type::i32:   [[fallthrough]];
    case r_attrib_type::ivec2: [[fallthrough]];
    case r_attrib_type::ivec3: [[fallthrough]];
    case r_attrib_type::ivec4: return GL_INT;

    case r_attrib_type::none:  return 0;
  }
  return 0;
}

constexpr inline GLenum gl_primitive_cast(r_primitive primitive) {
  switch (primitive) {
    case r_primitive::points:         return GL_POINTS;
    case r_primitive::triangles:      return GL_TRIANGLES;
    case r_primitive::triangle_fan:   return GL_TRIANGLE_FAN;
    case r_primitive::lines:          return GL_LINES;
    case r_primitive::line_strip:     return GL_LINE_STRIP;
    case r_primitive::triangle_strip: return GL_TRIANGLE_STRIP;

    case r_primitive::none:           return 0;
  }
  return 0;
}

constexpr inline bool gl_validate_descriptor(const r_attrib_descriptor&) {
  return true;
}

constexpr inline bool gl_validate_descriptor(const r_texture_descriptor& desc) {
  // TODO: Validate dimensions
  return 
    desc.count > 0 &&
    desc.type != r_texture_type::none &&
    !(desc.count > 1 && desc.type == r_texture_type::texture3d) &&
    !(desc.count != 6 && desc.type == r_texture_type::cubemap);
}

constexpr inline bool gl_validate_descriptor(const r_buffer_descriptor&) {
  return true;
}

constexpr inline bool gl_validate_descriptor(const r_pipeline_descriptor& desc) {
  return !(!desc.stages || desc.stage_count < 2);
}

constexpr inline bool gl_validate_descriptor(const r_shader_descriptor&) {
  return true;
}

constexpr inline bool gl_validate_descriptor(const r_framebuffer_descriptor&) {
  return true;
}

} // namespace ntf
