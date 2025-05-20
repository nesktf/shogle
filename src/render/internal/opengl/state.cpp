#include "./context.hpp"
#include "../../attributes.hpp"

namespace ntf {

GLenum gl_state::attrib_underlying_type_cast(r_attrib_type type) noexcept {
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
  }

  NTF_UNREACHABLE();
}

GLbitfield gl_state::clear_bit_cast(r_clear_flag flags) noexcept {
  GLbitfield clear_bits{0};
  if (+(flags & r_clear_flag::color)) {
    clear_bits |= GL_COLOR_BUFFER_BIT;
  }
  if (+(flags & r_clear_flag::depth)) {
    clear_bits |= GL_DEPTH_BUFFER_BIT;
  }
  if (+(flags & r_clear_flag::stencil)) {
    clear_bits |= GL_STENCIL_BUFFER_BIT;
  }
  return clear_bits;
}

GLenum gl_state::check_error(const char* file, const char* func, int line) noexcept {
  GLenum out = GL_NO_ERROR;
  GLenum err{};
  while ((err = glGetError()) != GL_NO_ERROR) {
    out = err;
    const char* err_str;
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
    SHOGLE_LOG(error, "[{}:{}][{}] GL ERROR ({}) -> {}",
               file, line, func, err, err_str);
  }
  return out;
}

gl_state::gl_state() noexcept :
  // _tex_limits{0, 0, 0},
  _bound_vao{0},
  _bound_program{0},
  _active_tex{0} {
  std::memset(_bound_buffers, NULL_BINDING, BUFFER_TYPE_COUNT*sizeof(GLuint));
  std::memset(_bound_fbos, DEFAULT_FBO, FBO_BIND_COUNT*sizeof(GLuint));
}

void gl_state::init(const init_data_t& data) noexcept {
  GLint max_tex;
  GL_CALL(glGetIntegerv, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tex);
  _bound_texs.resize(max_tex, std::make_pair(NULL_BINDING, GL_TEXTURE_2D));

  GLint max_tex_lay;
  GL_CALL(glGetIntegerv, GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_lay);
  _tex_limits.max_layers = max_tex_lay;

  GLint max_tex_dim;
  GL_CALL(glGetIntegerv, GL_MAX_TEXTURE_SIZE, &max_tex_dim);
  _tex_limits.max_dim = max_tex_dim;

  GLint max_tex_dim3d;
  GL_CALL(glGetIntegerv, GL_MAX_3D_TEXTURE_SIZE, &max_tex_dim3d);
  _tex_limits.max_dim3d = max_tex_dim3d;

  // GLint max_fbo_attach;
  // glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_fbo_attach);
  // _fbo_max_attachments = max_fbo_attach;

  // State cleanup
  GL_CALL(glBindFramebuffer, GL_FRAMEBUFFER, DEFAULT_FBO);
  GL_CALL(glUseProgram, NULL_BINDING);
  GL_CALL(glBindVertexArray, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_ARRAY_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_UNIFORM_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_TEXTURE_BUFFER, NULL_BINDING);
  GL_CALL(glBindBuffer, GL_SHADER_STORAGE_BUFFER, NULL_BINDING);
  for (GLint i = max_tex-1; i >= 0; --i) {
    // Do it in reverse to end up with GL_TEXTURE0 active
    GL_CALL(glActiveTexture, GL_TEXTURE0+i);
    GL_CALL(glBindTexture, GL_TEXTURE_2D, NULL_BINDING);
  }

  if (data.dbg) {
    GL_CALL(glEnable, GL_DEBUG_OUTPUT);
    GL_CALL(glDebugMessageCallback, data.dbg, &data.ctx);
  }
  GL_CALL(glEnable, GL_DEPTH_TEST);
}

auto gl_state::create_vao() noexcept -> vao_t {
  GLuint id;
  GL_CALL(glGenVertexArrays, 1, &id);
  vao_t vao;
  vao.id = id;
  return vao;
}

void gl_state::bind_vao(GLuint id) noexcept {
  if (_bound_vao == id) {
    return;
  }
  GL_CALL(glBindVertexArray, id);
  _bound_vao = id;
}

void gl_state::destroy_vao(const vao_t& vao) noexcept {
  NTF_ASSERT(vao.id);
  GLuint id = vao.id;
  if (_bound_vao == id) {
    _bound_vao = NULL_BINDING;
    GL_CALL(glBindVertexArray, NULL_BINDING);
  }
  GL_CALL(glDeleteVertexArrays, 1, &id);
}

void gl_state::bind_attributes(const r_attrib_descriptor* attrs, uint32 count, 
                               size_t stride) noexcept {
  NTF_ASSERT(attrs);
  NTF_ASSERT(count > 0);
  NTF_ASSERT(stride > 0);
  for (uint32 i = 0; i < count; ++i) {
    const auto& attr = attrs[i];
    // TODO: Don't re-enable already enabled attribs (and disable others)
    GL_CALL(glEnableVertexAttribArray, attr.location);

    const uint32 type_dim = r_attrib_type_dim(attr.type);
    NTF_ASSERT(type_dim);
    const GLenum gl_underlying_type = gl_state::attrib_underlying_type_cast(attr.type);
    NTF_ASSERT(gl_underlying_type);
    GL_CALL(glVertexAttribPointer,
      attr.location,
      type_dim,
      gl_underlying_type,
      GL_FALSE, // Don't normalize
      stride,
      reinterpret_cast<void*>(attr.offset)
    );
  }
}

void gl_state::prepare_draw_target(GLuint fb, r_clear_flag flags,
                                   const uvec4& vp, const color4& col) noexcept {
  bind_framebuffer(fb, gl_state::FBO_BIND_WRITE);
  GL_CALL(glViewport, vp.x, vp.y, vp.z, vp.w);
  if (!+(flags & r_clear_flag::color)) {
    return;
  }
  GL_CALL(glClearColor, col.r, col.g, col.b, col.a);
  GL_CALL(glClear, clear_bit_cast(flags));
}

void gl_state::prepare_external_state(const r_external_state& state) {
  const auto poly_mode = poly_mode_cast(state.poly_mode);
  GL_CALL(glPolygonMode, GL_FRONT_AND_BACK, poly_mode);
  if (poly_mode == GL_LINE) {
    GL_CALL(glLineWidth, state.poly_width.value_or(1.f));
  } else {
    GL_CALL(glPointSize, state.poly_width.value_or(1.f));
  }

  if (state.depth_test) {
    const auto& dp = state.depth_test;
    GL_CALL(glEnable, GL_DEPTH_TEST);
    GL_CALL(glDepthFunc, test_func_cast(dp->test_func));
    GL_CALL(glDepthRange, dp->near_bound, dp->far_bound);
  } else {
    GL_CALL(glDisable, GL_DEPTH_TEST);
  }

  if (state.stencil_test) {
    const auto& st = state.stencil_test;
    const auto& func = state.stencil_test->stencil_func;
    GL_CALL(glEnable, GL_STENCIL_TEST);
    GL_CALL(glStencilFunc, test_func_cast(func.func), func.ref, func.mask);

    const auto& rule = st->stencil_rule;
    GL_CALL(glStencilOp,
            stencil_op_cast(rule.on_stencil_fail),
            stencil_op_cast(rule.on_depth_fail),
            stencil_op_cast(rule.on_pass));
    GL_CALL(glStencilMask, st->stencil_mask);
  } else {
    GL_CALL(glDisable, GL_STENCIL_TEST);
  }

  if (state.blending) {
    const auto& bl = state.blending;
    GL_CALL(glEnable, GL_BLEND);
    GL_CALL(glBlendEquation, blend_eq_cast(bl->mode));
    GL_CALL(glBlendFunc, blend_func_cast(bl->src_factor), blend_func_cast(bl->dst_factor));
    GL_CALL(glBlendColor, bl->color.r, bl->color.g, bl->color.b, bl->color.a);
  } else {
    GL_CALL(glDisable, GL_BLEND);
  }

  if (state.scissor_test) {
    const auto& sc = state.scissor_test;
    GL_CALL(glEnable, GL_SCISSOR_TEST);
    GL_CALL(glScissor, sc->pos.x, sc->pos.y, sc->size.x, sc->size.y);
  } else {
    GL_CALL(glDisable, GL_SCISSOR_TEST);
  }

  if (state.face_culling) {
    const auto& cu = state.face_culling;
    GL_CALL(glEnable, GL_CULL_FACE);
    GL_CALL(glCullFace, cull_mode_cast(cu->mode));
    GL_CALL(glFrontFace, cull_face_cast(cu->front_face));
  } else {
    GL_CALL(glDisable, GL_CULL_FACE);
  }
}

} // namespace ntf
