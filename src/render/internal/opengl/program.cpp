#include "./state.hpp"

namespace ntf {

r_attrib_type gl_state::uniform_type_cast(GLenum type) noexcept {
  // TODO: Handle all (or most) sampler types
  switch (type) {
    case GL_FLOAT: return r_attrib_type::f32;
    case GL_FLOAT_VEC2: return r_attrib_type::vec2;
    case GL_FLOAT_VEC3: return r_attrib_type::vec3;
    case GL_FLOAT_VEC4: return r_attrib_type::vec4;
    case GL_FLOAT_MAT3: return r_attrib_type::mat3;
    case GL_FLOAT_MAT4: return r_attrib_type::mat4;

    case GL_DOUBLE: return r_attrib_type::f64;
    case GL_DOUBLE_VEC2: return r_attrib_type::dvec2;
    case GL_DOUBLE_VEC3: return r_attrib_type::dvec3;
    case GL_DOUBLE_VEC4: return r_attrib_type::dvec4;
    case GL_DOUBLE_MAT3: return r_attrib_type::dmat3;
    case GL_DOUBLE_MAT4: return r_attrib_type::dmat4;

    case GL_SAMPLER_1D: [[fallthrough]];
    case GL_SAMPLER_2D: [[fallthrough]];
    case GL_SAMPLER_3D: [[fallthrough]];
    case GL_SAMPLER_CUBE: [[fallthrough]];
    case GL_INT: return r_attrib_type::i32;

    case GL_INT_VEC2: return r_attrib_type::ivec2;
    case GL_INT_VEC3: return r_attrib_type::ivec3;
    case GL_INT_VEC4: return r_attrib_type::ivec4;
    
    default: NTF_UNREACHABLE();
  }
}

GLenum gl_state::primitive_cast(r_primitive primitive) noexcept {
  switch (primitive) {
    case r_primitive::points:         return GL_POINTS;
    case r_primitive::triangles:      return GL_TRIANGLES;
    case r_primitive::triangle_fan:   return GL_TRIANGLE_FAN;
    case r_primitive::lines:          return GL_LINES;
    case r_primitive::line_strip:     return GL_LINE_STRIP;
    case r_primitive::triangle_strip: return GL_TRIANGLE_STRIP;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::poly_mode_cast(r_polygon_mode poly_mode) noexcept {
  switch (poly_mode) {
    case r_polygon_mode::line: return GL_LINE;
    case r_polygon_mode::fill: return GL_FILL;
    case r_polygon_mode::point: return GL_POINT;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::test_func_cast(r_test_func func) noexcept {
  switch (func) {
    case r_test_func::never: return GL_NEVER;
    case r_test_func::always: return GL_ALWAYS;
    case r_test_func::less: return GL_LESS;
    case r_test_func::equal: return GL_EQUAL;
    case r_test_func::lequal: return GL_LEQUAL;
    case r_test_func::greater: return GL_GREATER;
    case r_test_func::nequal: return GL_NOTEQUAL;
    case r_test_func::gequal: return GL_GEQUAL;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::stencil_op_cast(r_stencil_op op) noexcept {
  switch (op) {
    case r_stencil_op::keep: return GL_KEEP;
    case r_stencil_op::set_zero: return GL_ZERO;
    case r_stencil_op::replace: return GL_REPLACE;
    case r_stencil_op::incr: return GL_INCR;
    case r_stencil_op::incr_wrap: return GL_INCR_WRAP;
    case r_stencil_op::decr: return GL_DECR;
    case r_stencil_op::decr_wrap: return GL_DECR_WRAP;
    case r_stencil_op::invert: return GL_INVERT;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::blend_func_cast(r_blend_factor func) noexcept {
  switch (func) {
    case r_blend_factor::zero: return GL_ZERO;
    case r_blend_factor::one: return GL_ONE;

    case r_blend_factor::src_color: return GL_SRC_COLOR;
    case r_blend_factor::inv_src_color: return GL_ONE_MINUS_SRC_COLOR;

    case r_blend_factor::dst_color: return GL_DST_COLOR;
    case r_blend_factor::inv_dst_color: return GL_ONE_MINUS_DST_COLOR;

    case r_blend_factor::src_alpha: return GL_SRC_ALPHA;
    case r_blend_factor::inv_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
    case r_blend_factor::src_alpha_sat: return GL_SRC_ALPHA_SATURATE;

    case r_blend_factor::dst_alpha: return GL_DST_ALPHA;
    case r_blend_factor::inv_dst_alpha: return GL_ONE_MINUS_DST_ALPHA;

    case r_blend_factor::const_color: return GL_CONSTANT_COLOR;
    case r_blend_factor::inv_const_color: return GL_ONE_MINUS_CONSTANT_COLOR;

    case r_blend_factor::const_alpha: return GL_CONSTANT_ALPHA;
    case r_blend_factor::inv_const_alpha: return GL_ONE_MINUS_CONSTANT_ALPHA;

    case r_blend_factor::src1_color: return GL_SRC1_COLOR;
    case r_blend_factor::inv_src1_color: return GL_ONE_MINUS_SRC1_COLOR;

    case r_blend_factor::src1_alpha: return GL_SRC1_ALPHA;
    case r_blend_factor::inv_src1_alpha: return GL_ONE_MINUS_SRC1_ALPHA;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::cull_mode_cast(r_cull_mode mode) noexcept {
  switch (mode) {
    case r_cull_mode::back: return GL_BACK;
    case r_cull_mode::front: return GL_FRONT;
    case r_cull_mode::front_back: return GL_FRONT_AND_BACK;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::cull_face_cast(r_cull_face face) noexcept {
  switch (face) {
    case r_cull_face::clockwise: return GL_CW;
    case r_cull_face::counter_clockwise: return GL_CCW;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::blend_eq_cast(r_blend_mode eq) noexcept {
  switch (eq) {
    case r_blend_mode::add: return GL_FUNC_ADD;
    case r_blend_mode::subs: return GL_FUNC_SUBTRACT;
    case r_blend_mode::rev_subs: return GL_FUNC_REVERSE_SUBTRACT;
    case r_blend_mode::min: return GL_MIN;
    case r_blend_mode::max: return GL_MAX;
  }
  NTF_UNREACHABLE();
}

auto gl_state::create_program(cspan<shader_t*> shaders,
                              r_primitive primitive,
                              r_polygon_mode poly_mode, optional<float> poly_width,
                              weak_cptr<r_stencil_test_opts> stencil,
                              weak_cptr<r_depth_test_opts> depth,
                              weak_cptr<r_scissor_test_opts> scissor,
                              weak_cptr<r_blend_opts> blending,
                              weak_cptr<r_face_cull_opts> culling) -> program_t {
  GLuint id = GL_CALL_RET(glCreateProgram);
  for (const auto& shader : shaders) {
    // TODO: Ensure vertex and fragment exist
    // TODO: Check for duplicate shader types
    GL_CALL(glAttachShader, id, shader->id);
  }
  GL_CALL(glLinkProgram, id);

  int succ;
  GL_CALL(glGetProgramiv, id, GL_LINK_STATUS, &succ);
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_CALL(glGetProgramiv, id, GL_INFO_LOG_LENGTH, &err_len);
    std::string log;
    log.resize(err_len);

    GL_CALL(glGetShaderInfoLog, shaders[0]->id, 1024, &err_len, log.data());
    GL_CALL(glDeleteProgram, id);
    throw error<>::format({"Failed to link program: {}"}, log);
  }

  for (const auto& shader : shaders) {
    GL_CALL(glDetachShader, id, shader->id);
  }

  program_t prog;
  prog.id = id;
  prog.primitive = primitive_cast(primitive);
  prog.poly = poly_mode_cast(poly_mode);
  prog.poly_width = poly_width.value_or(1.f);
  update_program(prog,
                 stencil, depth, scissor, blending, culling);

  return prog;
}

void gl_state::update_program(program_t& prog,
                              weak_cptr<r_stencil_test_opts> stencil,
                              weak_cptr<r_depth_test_opts> depth,
                              weak_cptr<r_scissor_test_opts> scissor,
                              weak_cptr<r_blend_opts> blending,
                              weak_cptr<r_face_cull_opts> culling) {
  if (depth) {
    prog.flags |= PROG_ENABLE_DEPTH;
    auto& dp = prog.depth;
    dp.func = test_func_cast(depth->test_func);
    dp.near = depth->near_bound;
    dp.far = depth->far_bound;
  } else {
    prog.flags &= ~PROG_ENABLE_DEPTH;
  }

  if (stencil) {
    prog.flags |= PROG_ENABLE_STENCIL;
    auto& st = prog.stencil;
    st.func = test_func_cast(stencil->stencil_func.func);
    st.func_ref = stencil->stencil_func.ref;
    st.func_mask = stencil->stencil_func.mask;
    st.sfail = stencil_op_cast(stencil->stencil_rule.on_stencil_fail);
    st.dpfail = stencil_op_cast(stencil->stencil_rule.on_depth_fail);
    st.dppass = stencil_op_cast(stencil->stencil_rule.on_pass);
    st.mask = stencil->stencil_mask;
  } else {
    prog.flags &= ~PROG_ENABLE_STENCIL;
  }

  if (blending) {
    prog.flags |= PROG_ENABLE_BLENDING;
    auto& bl = prog.blending;
    bl.mode = blend_eq_cast(blending->mode);
    bl.src_fac = blend_func_cast(blending->src_factor);
    bl.dst_fac = blend_func_cast(blending->dst_factor);
    bl.r = blending->color.r;
    bl.g = blending->color.g;
    bl.b = blending->color.b;
    bl.a = blending->color.a;
  } else {
    prog.flags &= ~PROG_ENABLE_BLENDING;
  }

  if (scissor) {
    prog.flags |= PROG_ENABLE_SCISSOR;
    auto& sc = prog.scissor;
    sc.x = scissor->pos.x;
    sc.y = scissor->pos.y;
    sc.w = scissor->size.x;
    sc.h = scissor->size.y;
  } else {
    prog.flags &= ~PROG_ENABLE_SCISSOR;
  }

  if (culling) {
    prog.flags |= PROG_ENABLE_CULLING;
    auto& cu = prog.culling;
    cu.mode = cull_mode_cast(culling->mode);
    cu.face = cull_face_cast(culling->front_face);
  } else {
    prog.flags &= ~PROG_ENABLE_CULLING;
  }
}

void gl_state::destroy_program(const program_t& prog) noexcept {
  NTF_ASSERT(prog.id);
  if (_bound_program == prog.id) {
    _bound_program = NULL_BINDING;
    GL_CALL(glUseProgram, NULL_BINDING);
  }
  GL_CALL(glDeleteProgram, prog.id);
}

bool gl_state::bind_program(GLuint id) noexcept {
  if (_bound_program == id) {
    return false;
  }
  GL_CALL(glUseProgram, id);
  _bound_program = id;
  return true;
}

bool gl_state::prepare_program(const program_t& prog) noexcept {
  bool rebind = bind_program(prog.id);

  GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, prog.poly);
  if (prog.poly == GL_LINE) {
    GL_CALL(glLineWidth, prog.poly_width);
  } else if (prog.poly == GL_POINT) {
    GL_CALL(glPointSize, prog.poly_width);
  }

  if (prog.flags & PROG_ENABLE_DEPTH) {
    const auto& dp = prog.depth;
    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glDepthFunc, dp.func);
    GL_CALL(glDepthRange, dp.near, dp.far);
  } else {
    GL_CALL(glDisable, GL_DEPTH_TEST);
  }

  if (prog.flags & PROG_ENABLE_STENCIL) {
    const auto& st = prog.stencil;
    GL_CALL(glEnable, GL_STENCIL_TEST);
    // TODO: Allow the user to decide if the test applies to the front face, or the back face
    GL_CALL(glStencilFunc, st.func, st.func_ref, st.func_mask);
    GL_CALL(glStencilOp, st.sfail, st.dpfail, st.dppass);
    GL_CALL(glStencilMask, st.mask);
  } else {
    GL_CALL(glDisable, GL_STENCIL_TEST);
  }

  if (prog.flags & PROG_ENABLE_BLENDING) {
    const auto& bl = prog.blending;
    GL_CALL(glEnable, GL_BLEND);
    GL_CALL(glBlendEquation, bl.mode);
    GL_CALL(glBlendFunc, bl.src_fac, bl.dst_fac);
    GL_CALL(glBlendColor, bl.r, bl.g, bl.b, bl.a);
  } else {
    GL_CALL(glDisable, GL_BLEND);
  }

  if (prog.flags & PROG_ENABLE_SCISSOR) {
    const auto& sc = prog.scissor;
    GL_CALL(glEnable, GL_SCISSOR_TEST);
    GL_CALL(glScissor, sc.x, sc.y, sc.w, sc.h);
  } else {
    GL_CALL(glDisable, GL_SCISSOR_TEST);
  }

  if (prog.flags & PROG_ENABLE_CULLING) {
    const auto& cu = prog.culling;
    GL_CALL(glEnable, GL_CULL_FACE);
    GL_CALL(glCullFace, cu.mode);
    GL_CALL(glFrontFace, cu.face);
  } else {
    GL_CALL(glDisable, GL_CULL_FACE);
  }

  return rebind;
}

void gl_state::push_uniform(uint32 loc, r_attrib_type type, const void* data) noexcept {
  NTF_ASSERT(data);
  switch (type) {
    case r_attrib_type::f32: {
      GL_CALL(glUniform1f, loc, *static_cast<const float*>(data));
      break;
    }
    case r_attrib_type::vec2: {
      GL_CALL(glUniform2fv, loc, 1, static_cast<const float*>(data));
      break;
    }
    case r_attrib_type::vec3: {
      GL_CALL(glUniform3fv, loc, 1, static_cast<const float*>(data));
      break;
    }
    case r_attrib_type::vec4: {
      GL_CALL(glUniform4fv, loc, 1, static_cast<const float*>(data));
      break;
    }
    case r_attrib_type::mat3: {
      GL_CALL(glUniformMatrix3fv, loc, 1, GL_FALSE, static_cast<const float*>(data));
      break;
    } 
    case r_attrib_type::mat4: {
      GL_CALL(glUniformMatrix4fv, loc, 1, GL_FALSE, static_cast<const float*>(data));
      break;
    }

    case r_attrib_type::f64: {
      GL_CALL(glUniform1d, loc, *static_cast<const double*>(data));
      break;
    }
    case r_attrib_type::dvec2: {
      GL_CALL(glUniform2dv, loc, 1, static_cast<const double*>(data));
      break;
    }
    case r_attrib_type::dvec3: {
      GL_CALL(glUniform3dv, loc, 1, static_cast<const double*>(data));
      break;
    }
    case r_attrib_type::dvec4: {
      GL_CALL(glUniform4dv, loc, 1, static_cast<const double*>(data));
      break;
    }
    case r_attrib_type::dmat3: {
      GL_CALL(glUniformMatrix3dv, loc, 1, GL_FALSE, static_cast<const double*>(data));
      break;
    } 
    case r_attrib_type::dmat4: {
      GL_CALL(glUniformMatrix4dv, loc, 1, GL_FALSE, static_cast<const double*>(data));
      break;
    }

    case r_attrib_type::i32: {
      GL_CALL(glUniform1i, loc, *static_cast<const int32*>(data));
      break;
    }
    case r_attrib_type::ivec2: {
      GL_CALL(glUniform2iv, loc, 1, static_cast<const int32*>(data));
      break;
    }
    case r_attrib_type::ivec3: {
      GL_CALL(glUniform3iv, loc, 1, static_cast<const int32*>(data));
      break;
    }
    case r_attrib_type::ivec4: {
      GL_CALL(glUniform4iv, loc, 1, static_cast<const int32*>(data));
      break;
    }

    default: {
      NTF_UNREACHABLE();
    }
  };
}

void gl_state::query_program_uniforms(const program_t& prog, rp_uniform_query_vec& unif) {
  NTF_ASSERT(unif.empty());
  int count;
  GL_CALL(glGetProgramiv, prog.id, GL_ACTIVE_UNIFORMS, &count);
  NTF_ASSERT(count);
  for (int i = 0; i < count; ++i) {
    GLint size;
    GLenum type;
    GLchar name[128];
    GLsizei len;
    GL_CALL(glGetActiveUniform, prog.id, static_cast<GLuint>(i), 128, &len, &size, &type, name);
    unif.emplace_back(rp_uniform_query{
      .location = r_platform_uniform{static_cast<r_handle_value>(i)},
      .name = name,
      .type = uniform_type_cast(type),
      .size = static_cast<size_t>(len),
    });
  }
}

} // namespace ntf
