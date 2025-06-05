#include "./context.hpp"

namespace ntf::render {

attribute_type gl_state::uniform_type_cast(GLenum type) noexcept {
  // TODO: Handle all (or most) sampler types
  switch (type) {
    case GL_FLOAT: return attribute_type::f32;
    case GL_FLOAT_VEC2: return attribute_type::vec2;
    case GL_FLOAT_VEC3: return attribute_type::vec3;
    case GL_FLOAT_VEC4: return attribute_type::vec4;
    case GL_FLOAT_MAT3: return attribute_type::mat3;
    case GL_FLOAT_MAT4: return attribute_type::mat4;

    case GL_DOUBLE: return attribute_type::f64;
    case GL_DOUBLE_VEC2: return attribute_type::dvec2;
    case GL_DOUBLE_VEC3: return attribute_type::dvec3;
    case GL_DOUBLE_VEC4: return attribute_type::dvec4;
    case GL_DOUBLE_MAT3: return attribute_type::dmat3;
    case GL_DOUBLE_MAT4: return attribute_type::dmat4;

    case GL_SAMPLER_1D: [[fallthrough]];
    case GL_SAMPLER_2D: [[fallthrough]];
    case GL_SAMPLER_3D: [[fallthrough]];
    case GL_SAMPLER_CUBE: [[fallthrough]];
    case GL_INT: return attribute_type::i32;

    case GL_INT_VEC2: return attribute_type::ivec2;
    case GL_INT_VEC3: return attribute_type::ivec3;
    case GL_INT_VEC4: return attribute_type::ivec4;
    
    default: NTF_UNREACHABLE();
  }
}

GLenum gl_state::primitive_cast(primitive_mode primitive) noexcept {
  switch (primitive) {
    case primitive_mode::points:         return GL_POINTS;
    case primitive_mode::triangles:      return GL_TRIANGLES;
    case primitive_mode::triangle_fan:   return GL_TRIANGLE_FAN;
    case primitive_mode::lines:          return GL_LINES;
    case primitive_mode::line_strip:     return GL_LINE_STRIP;
    case primitive_mode::triangle_strip: return GL_TRIANGLE_STRIP;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::poly_mode_cast(polygon_mode poly_mode) noexcept {
  switch (poly_mode) {
    case polygon_mode::line: return GL_LINE;
    case polygon_mode::fill: return GL_FILL;
    case polygon_mode::point: return GL_POINT;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::test_func_cast(test_func func) noexcept {
  switch (func) {
    case test_func::never: return GL_NEVER;
    case test_func::always: return GL_ALWAYS;
    case test_func::less: return GL_LESS;
    case test_func::equal: return GL_EQUAL;
    case test_func::lequal: return GL_LEQUAL;
    case test_func::greater: return GL_GREATER;
    case test_func::nequal: return GL_NOTEQUAL;
    case test_func::gequal: return GL_GEQUAL;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::stencil_op_cast(stencil_op op) noexcept {
  switch (op) {
    case stencil_op::keep: return GL_KEEP;
    case stencil_op::set_zero: return GL_ZERO;
    case stencil_op::replace: return GL_REPLACE;
    case stencil_op::incr: return GL_INCR;
    case stencil_op::incr_wrap: return GL_INCR_WRAP;
    case stencil_op::decr: return GL_DECR;
    case stencil_op::decr_wrap: return GL_DECR_WRAP;
    case stencil_op::invert: return GL_INVERT;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::blend_func_cast(blend_factor func) noexcept {
  switch (func) {
    case blend_factor::zero: return GL_ZERO;
    case blend_factor::one: return GL_ONE;

    case blend_factor::src_color: return GL_SRC_COLOR;
    case blend_factor::inv_src_color: return GL_ONE_MINUS_SRC_COLOR;

    case blend_factor::dst_color: return GL_DST_COLOR;
    case blend_factor::inv_dst_color: return GL_ONE_MINUS_DST_COLOR;

    case blend_factor::src_alpha: return GL_SRC_ALPHA;
    case blend_factor::inv_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
    case blend_factor::src_alpha_sat: return GL_SRC_ALPHA_SATURATE;

    case blend_factor::dst_alpha: return GL_DST_ALPHA;
    case blend_factor::inv_dst_alpha: return GL_ONE_MINUS_DST_ALPHA;

    case blend_factor::const_color: return GL_CONSTANT_COLOR;
    case blend_factor::inv_const_color: return GL_ONE_MINUS_CONSTANT_COLOR;

    case blend_factor::const_alpha: return GL_CONSTANT_ALPHA;
    case blend_factor::inv_const_alpha: return GL_ONE_MINUS_CONSTANT_ALPHA;

    case blend_factor::src1_color: return GL_SRC1_COLOR;
    case blend_factor::inv_src1_color: return GL_ONE_MINUS_SRC1_COLOR;

    case blend_factor::src1_alpha: return GL_SRC1_ALPHA;
    case blend_factor::inv_src1_alpha: return GL_ONE_MINUS_SRC1_ALPHA;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::cull_mode_cast(cull_mode mode) noexcept {
  switch (mode) {
    case cull_mode::back: return GL_BACK;
    case cull_mode::front: return GL_FRONT;
    case cull_mode::front_back: return GL_FRONT_AND_BACK;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::cull_face_cast(cull_face face) noexcept {
  switch (face) {
    case cull_face::clockwise: return GL_CW;
    case cull_face::counter_clockwise: return GL_CCW;
  }
  NTF_UNREACHABLE();
}

GLenum gl_state::blend_eq_cast(blend_mode eq) noexcept {
  switch (eq) {
    case blend_mode::add: return GL_FUNC_ADD;
    case blend_mode::subs: return GL_FUNC_SUBTRACT;
    case blend_mode::rev_subs: return GL_FUNC_REVERSE_SUBTRACT;
    case blend_mode::min: return GL_MIN;
    case blend_mode::max: return GL_MAX;
  }
  NTF_UNREACHABLE();
}

ctx_pip_status gl_state::create_program(glprog_t& prog, cspan<glshader_t*> shaders,
                                        primitive_mode primitive,
                                        polygon_mode poly_mode, f32 poly_width,
                                        render_tests tests, pip_err_str& err)
{
  GLuint id = GL_CALL_RET(glCreateProgram);
  for (const auto& shader : shaders) {
    // TODO: Ensure vertex and fragment exist
    // TODO: Check for duplicate shader types
    GL_CALL(glAttachShader, id, shader->id);
  }
  glLinkProgram(id); // avoid asserts

  int succ;
  GL_CALL(glGetProgramiv, id, GL_LINK_STATUS, &succ);
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_CALL(glGetProgramiv, id, GL_INFO_LOG_LENGTH, &err_len);
    auto span = _alloc.arena_span<char>(static_cast<size_t>(err_len));
    GL_CALL(glGetShaderInfoLog, shaders[0]->id, 1024, &err_len, span.data());
    GL_CALL(glDeleteProgram, id);
    err = {span.data(), span.size()};
    RENDER_ERROR_LOG("Failed to link program: {}", err);
    return CTX_PIP_STATUS_LINKING_FAILED;
  }

  for (const auto& shader : shaders) {
    GL_CALL(glDetachShader, id, shader->id);
  }

  prog.id = id;
  prog.primitive = primitive_cast(primitive);
  prog.poly = poly_mode_cast(poly_mode);
  prog.poly_width = poly_width;

  if (tests.depth_test) {
    const auto depth = *tests.depth_test;
    prog.flags |= GLPROG_ENABLE_DEPTH;
    auto& dp = prog.depth;
    dp.func = test_func_cast(depth.func);
    dp.near = depth.near_bound;
    dp.far = depth.far_bound;
  } else {
    prog.flags &= ~GLPROG_ENABLE_DEPTH;
  }

  if (tests.stencil_test) {
    const auto stencil = *tests.stencil_test;
    prog.flags |= GLPROG_ENABLE_STENCIL;
    auto& st = prog.stencil;
    st.func = test_func_cast(stencil.func.func);
    st.func_ref = stencil.func.ref;
    st.func_mask = stencil.func.mask;
    st.sfail = stencil_op_cast(stencil.rule.on_stencil_fail);
    st.dpfail = stencil_op_cast(stencil.rule.on_depth_fail);
    st.dppass = stencil_op_cast(stencil.rule.on_pass);
    st.mask = stencil.mask;
  } else {
    prog.flags &= ~GLPROG_ENABLE_STENCIL;
  }

  if (tests.blending) {
    const auto blending = *tests.blending;
    prog.flags |= GLPROG_ENABLE_BLENDING;
    auto& bl = prog.blending;
    bl.mode = blend_eq_cast(blending.mode);
    bl.src_fac = blend_func_cast(blending.src_factor);
    bl.dst_fac = blend_func_cast(blending.dst_factor);
    bl.r = blending.color.r;
    bl.g = blending.color.g;
    bl.b = blending.color.b;
    bl.a = blending.color.a;
  } else {
    prog.flags &= ~GLPROG_ENABLE_BLENDING;
  }

  if (tests.scissor_test) {
    const auto scissor = *tests.scissor_test;
    prog.flags |= GLPROG_ENABLE_SCISSOR;
    auto& sc = prog.scissor;
    sc.x = scissor.pos.x;
    sc.y = scissor.pos.y;
    sc.w = scissor.size.x;
    sc.h = scissor.size.y;
  } else {
    prog.flags &= ~GLPROG_ENABLE_SCISSOR;
  }

  if (tests.face_culling) {
    const auto culling = *tests.face_culling;
    prog.flags |= GLPROG_ENABLE_CULLING;
    auto& cu = prog.culling;
    cu.mode = cull_mode_cast(culling.mode);
    cu.face = cull_face_cast(culling.front_face);
  } else {
    prog.flags &= ~GLPROG_ENABLE_CULLING;
  }

  return CTX_PIP_STATUS_OK;
}

void gl_state::destroy_program(glprog_t& prog) {
  NTF_ASSERT(prog.id);
  if (_bound_program == prog.id) {
    _bound_program = NULL_BINDING;
    GL_CALL(glUseProgram, NULL_BINDING);
  }
  GL_CALL(glDeleteProgram, prog.id);
}

bool gl_state::program_bind(GLuint id) {
  if (_bound_program == id) {
    return false;
  }
  GL_CALL(glUseProgram, id);
  _bound_program = id;
  return true;
}

bool gl_state::program_prepare_state(glprog_t& prog) {
  bool rebind = program_bind(prog.id);

  GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, prog.poly);
  if (prog.poly == GL_LINE) {
    GL_CALL(glLineWidth, prog.poly_width);
  } else if (prog.poly == GL_POINT) {
    GL_CALL(glPointSize, prog.poly_width);
  }

  if (prog.flags & GLPROG_ENABLE_DEPTH) {
    const auto& dp = prog.depth;
    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glDepthFunc, dp.func);
    GL_CALL(glDepthRange, dp.near, dp.far);
  } else {
    GL_CALL(glDisable, GL_DEPTH_TEST);
  }

  if (prog.flags & GLPROG_ENABLE_STENCIL) {
    const auto& st = prog.stencil;
    GL_CALL(glEnable, GL_STENCIL_TEST);
    // TODO: Allow the user to decide if the test applies to the front face, or the back face
    GL_CALL(glStencilFunc, st.func, st.func_ref, st.func_mask);
    GL_CALL(glStencilOp, st.sfail, st.dpfail, st.dppass);
    GL_CALL(glStencilMask, st.mask);
  } else {
    GL_CALL(glDisable, GL_STENCIL_TEST);
  }

  if (prog.flags & GLPROG_ENABLE_BLENDING) {
    const auto& bl = prog.blending;
    GL_CALL(glEnable, GL_BLEND);
    GL_CALL(glBlendEquation, bl.mode);
    GL_CALL(glBlendFunc, bl.src_fac, bl.dst_fac);
    GL_CALL(glBlendColor, bl.r, bl.g, bl.b, bl.a);
  } else {
    GL_CALL(glDisable, GL_BLEND);
  }

  if (prog.flags & GLPROG_ENABLE_SCISSOR) {
    const auto& sc = prog.scissor;
    GL_CALL(glEnable, GL_SCISSOR_TEST);
    GL_CALL(glScissor, sc.x, sc.y, sc.w, sc.h);
  } else {
    GL_CALL(glDisable, GL_SCISSOR_TEST);
  }

  if (prog.flags & GLPROG_ENABLE_CULLING) {
    const auto& cu = prog.culling;
    GL_CALL(glEnable, GL_CULL_FACE);
    GL_CALL(glCullFace, cu.mode);
    GL_CALL(glFrontFace, cu.face);
  } else {
    GL_CALL(glDisable, GL_CULL_FACE);
  }

  return rebind;
}

void gl_state::push_uniform(GLuint loc, attribute_type type, const void* data) {
  NTF_ASSERT(data);
  switch (type) {
    case attribute_type::f32: {
      GL_CALL(glUniform1f, loc, *static_cast<const float*>(data));
      break;
    }
    case attribute_type::vec2: {
      GL_CALL(glUniform2fv, loc, 1, static_cast<const float*>(data));
      break;
    }
    case attribute_type::vec3: {
      GL_CALL(glUniform3fv, loc, 1, static_cast<const float*>(data));
      break;
    }
    case attribute_type::vec4: {
      GL_CALL(glUniform4fv, loc, 1, static_cast<const float*>(data));
      break;
    }
    case attribute_type::mat3: {
      GL_CALL(glUniformMatrix3fv, loc, 1, GL_FALSE, static_cast<const float*>(data));
      break;
    } 
    case attribute_type::mat4: {
      GL_CALL(glUniformMatrix4fv, loc, 1, GL_FALSE, static_cast<const float*>(data));
      break;
    }

    case attribute_type::f64: {
      GL_CALL(glUniform1d, loc, *static_cast<const double*>(data));
      break;
    }
    case attribute_type::dvec2: {
      GL_CALL(glUniform2dv, loc, 1, static_cast<const double*>(data));
      break;
    }
    case attribute_type::dvec3: {
      GL_CALL(glUniform3dv, loc, 1, static_cast<const double*>(data));
      break;
    }
    case attribute_type::dvec4: {
      GL_CALL(glUniform4dv, loc, 1, static_cast<const double*>(data));
      break;
    }
    case attribute_type::dmat3: {
      GL_CALL(glUniformMatrix3dv, loc, 1, GL_FALSE, static_cast<const double*>(data));
      break;
    } 
    case attribute_type::dmat4: {
      GL_CALL(glUniformMatrix4dv, loc, 1, GL_FALSE, static_cast<const double*>(data));
      break;
    }

    case attribute_type::i32: {
      GL_CALL(glUniform1i, loc, *static_cast<const int32*>(data));
      break;
    }
    case attribute_type::ivec2: {
      GL_CALL(glUniform2iv, loc, 1, static_cast<const int32*>(data));
      break;
    }
    case attribute_type::ivec3: {
      GL_CALL(glUniform3iv, loc, 1, static_cast<const int32*>(data));
      break;
    }
    case attribute_type::ivec4: {
      GL_CALL(glUniform4iv, loc, 1, static_cast<const int32*>(data));
      break;
    }

    default: {
      NTF_UNREACHABLE();
    }
  };
}

void gl_state::program_query_uniforms(glprog_t& prog, unif_meta_vec& unif) {
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
    unif.emplace_back(ctx_unif_meta{
      .handle = static_cast<ctx_unif>(i),
      .name = {name, _alloc.make_adaptor<char>()},
      .type = uniform_type_cast(type),
      .size = static_cast<size_t>(len),
    });
  }
}

void gl_state::attribute_bind(cspan<attribute_binding> attribs) { 
  NTF_ASSERT(!attribs.empty());
  for (const auto& attr : attribs) {
    // TODO: Don't re-enable already enabled attribs (and disable others)
    GL_CALL(glEnableVertexAttribArray, attr.location);

    const uint32 type_dim = meta::attribute_dim(attr.type);
    NTF_ASSERT(type_dim);
    const GLenum gl_underlying_type = gl_state::attrib_underlying_type_cast(attr.type);
    NTF_ASSERT(gl_underlying_type);
    GL_CALL(glVertexAttribPointer,
      attr.location, type_dim, gl_underlying_type, GL_FALSE,
      attr.stride, reinterpret_cast<void*>(attr.offset));
  }
}

void gl_state::prepare_state(const external_state& state) {
  const auto poly_mode = poly_mode_cast(state.poly_mode);
  GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, poly_mode);
  if (poly_mode == GL_LINE) {
    GL_CALL(glLineWidth, state.poly_width);
  } else {
    GL_CALL(glPointSize, state.poly_width);
  }

  if (state.test.depth_test) {
    const auto dp = *state.test.depth_test;
    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glDepthFunc, test_func_cast(dp.func));
    GL_CALL(glDepthRange, dp.near_bound, dp.far_bound);
  } else {
    GL_CALL(glDisable, GL_DEPTH_TEST);
  }

  if (state.test.stencil_test) {
    const auto st = *state.test.stencil_test;
    const auto& func = st.func;
    GL_CALL(glEnable, GL_STENCIL_TEST);
    GL_CALL(glStencilFunc, test_func_cast(func.func), func.ref, func.mask);

    const auto& rule = st.rule;
    GL_CALL(glStencilOp,
            stencil_op_cast(rule.on_stencil_fail),
            stencil_op_cast(rule.on_depth_fail),
            stencil_op_cast(rule.on_pass));
    GL_CALL(glStencilMask, st.mask);
  } else {
    GL_CALL(glDisable, GL_STENCIL_TEST);
  }

  if (state.test.blending) {
    const auto bl = *state.test.blending;
    GL_CALL(glEnable, GL_BLEND);
    GL_CALL(glBlendEquation, blend_eq_cast(bl.mode));
    GL_CALL(glBlendFunc, blend_func_cast(bl.src_factor), blend_func_cast(bl.dst_factor));
    GL_CALL(glBlendColor, bl.color.r, bl.color.g, bl.color.b, bl.color.a);
  } else {
    GL_CALL(glDisable, GL_BLEND);
  }

  if (state.test.scissor_test) {
    const auto sc = *state.test.scissor_test;
    GL_CALL(glEnable, GL_SCISSOR_TEST);
    GL_CALL(glScissor, sc.pos.x, sc.pos.y, sc.size.x, sc.size.y);
  } else {
    GL_CALL(glDisable, GL_SCISSOR_TEST);
  }

  if (state.test.face_culling) {
    const auto cu = *state.test.face_culling;
    GL_CALL(glEnable, GL_CULL_FACE);
    GL_CALL(glCullFace, cull_mode_cast(cu.mode));
    GL_CALL(glFrontFace, cull_face_cast(cu.front_face));
  } else {
    GL_CALL(glDisable, GL_CULL_FACE);
  }
}

ctx_pip_status gl_context::create_pipeline(ctx_pip& pip, pip_err_str& err,
                                           const ctx_pip_desc& desc){
  NTF_ASSERT(!desc.stages.empty());
  auto shads = _alloc.arena_span<glshader_t*>(desc.stages.size());
  NTF_ASSERT(!shads.empty())
  for (size_t i = 0u; const ctx_shad stage : desc.stages) {
    auto& shader = _shaders.get(stage);
    shads[i] = &shader;
    ++i;
  }

  ctx_pip handle = _programs.acquire();
  NTF_ASSERT(check_handle(handle));
  auto& program = _programs.get(handle);
  const auto status = _state.create_program(program, shads, desc.primitive,
                                            desc.poly_mode, desc.poly_width,
                                            desc.tests, err);
  if (status != CTX_PIP_STATUS_OK) {
    _programs.push(handle);
    return status;
  }
  NTF_ASSERT(program.id);
  pip = handle;
  return status;
}

ctx_pip_status gl_context::destroy_pipeline(ctx_pip pip) noexcept {
  if (!_programs.validate(pip)) {
    return CTX_PIP_STATUS_INVALID_HANDLE;
  }
  auto& program = _programs.get(pip);
  _state.destroy_program(program);
  _programs.push(pip);
  return CTX_PIP_STATUS_OK;
}

} // namespace ntf::render
