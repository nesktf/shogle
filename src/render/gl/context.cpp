#include "./context_private.hpp"
#include <shogle/render/gl/context.hpp>

namespace shogle {

sv_expect<gl_context> gl_context::create(gl_surface_provider& surf_prov) noexcept {
  try {
    surf_prov.set_current_thread_context(); // Make sure the context is loaded in this thread
    auto ctx = std::make_unique<gl_private>();
#if defined(SHOGLE_USE_SYSTEM_GL) && SHOGLE_USE_SYSTEM_GL
    ctx->version_string = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!ctx->version_string) {
      return {ntf::unexpect, "Failed to retrieve OpenGL version"};
    }
    shogle_gl_get_version(ctx->version_string, &ctx->ver);
    ctx->vendor_string = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    ctx->renderer_string = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
#else
    const auto proc = surf_prov.proc_loader();
    if (!proc) {
      return {ntf::unexpect, "Invalid glGetProcAddress function"};
    }
    auto err = shogle_gl_load_funcs((PFN_shogle_glGetProcAddress)proc, &ctx->funcs, &ctx->ver);
    if (err) {
      static constexpr auto errors = std::to_array<const char*>(
        {"Failed to load OpenGL functions", "Failed to load OpenGL", "Invalid OpenGL version"});
      return {ntf::unexpect, errors[static_cast<u32>(err) - 1]};
    }
    ctx->version_string = reinterpret_cast<const char*>(ctx->funcs.glGetString(GL_VERSION));
    ctx->vendor_string = reinterpret_cast<const char*>(ctx->funcs.glGetString(GL_VENDOR));
    ctx->renderer_string = reinterpret_cast<const char*>(ctx->funcs.glGetString(GL_RENDERER));
#endif
    NTF_ASSERT(ctx->version_string);
    NTF_ASSERT(ctx->vendor_string);
    NTF_ASSERT(ctx->renderer_string);

    return {ntf::in_place, create_t{}, std::move(ctx), surf_prov};
  } catch (...) {
    return {ntf::unexpect, "Failed to allocate OpenGL context"};
  }
}

gl_context::gl_context(create_t, std::unique_ptr<gl_private>&& ctx,
                       gl_surface_provider& surf_prov) noexcept :
    _ctx(std::move(ctx)), _surf_prov(surf_prov) {}

gl_context::gl_context(gl_surface_provider& surf_prov) :
    gl_context(::shogle::gl_context::create(surf_prov).value()) {}

const gl_private& gl_context::internal() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return *_ctx;
}

gl_surface_provider& gl_context::provider() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return _surf_prov.get();
}

gl_version gl_context::version() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return gl_version{.major = static_cast<u32>(_ctx->ver.maj),
                    .minor = static_cast<u32>(_ctx->ver.min)};
}

std::string_view gl_context::renderer_string() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return _ctx->renderer_string;
}

std::string_view gl_context::vendor_string() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return _ctx->vendor_string;
}

std::string_view gl_context::version_string() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return _ctx->version_string;
}

gldefs::GLenum gl_context::get_error() const {
  gldefs::GLenum out = 0;
  gldefs::GLenum err;
  const auto& funcs = internal().funcs;
  while ((err = funcs.glGetError()) != GL_NO_ERROR) {
    out = err;
  }
  return out;
}

namespace {

constexpr GLuint DEFAULT_FRAMEBUFFER = 0;

void setup_framebuffer(gl_context& gl, GLuint fbo, const square_pos<u32>& viewport,
                       const square_pos<u32>& scissor) {
  GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));
  GL_ASSERT(glViewport(viewport.x, viewport.y, viewport.w, viewport.h));
  GL_ASSERT(glEnable(GL_SCISSOR_TEST));
  GL_ASSERT(glScissor(scissor.x, scissor.y, scissor.w, scissor.h));
}

void setup_render_state(gl_context& gl, const gl_pipeline::depth_test_props& depth_test,
                        const gl_pipeline::stencil_test_props& stencil_test,
                        const gl_pipeline::blending_props& blending,
                        const gl_pipeline::culling_props& culling, GLenum poly_mode,
                        f32 poly_width) {
  GL_ASSERT(glPolygonMode(GL_FRONT_AND_BACK, poly_mode));
  if (poly_mode == GL_LINE) {
    GL_ASSERT(glLineWidth(poly_width));
  } else {
    GL_ASSERT(glPointSize(poly_width));
  }

  if (depth_test.enable) {
    GL_ASSERT(glEnable(GL_DEPTH_TEST));
    GL_ASSERT(glDepthFunc(depth_test.func));
    GL_ASSERT(glDepthRange(depth_test.near, depth_test.far));
  } else {
    GL_ASSERT(glDisable(GL_DEPTH_TEST));
  }

  if (stencil_test.enable) {
    GL_ASSERT(glEnable(GL_STENCIL_TEST));
    GL_ASSERT(glStencilFunc(stencil_test.func, stencil_test.func_ref, stencil_test.func_mask));
    GL_ASSERT(glStencilOp(stencil_test.sfail, stencil_test.dpfail, stencil_test.dppass));
    GL_ASSERT(glStencilMask(stencil_test.mask));
  } else {
    GL_ASSERT(glDisable(GL_STENCIL_TEST));
  }

  if (blending.enable) {
    GL_ASSERT(glEnable(GL_BLEND));
    GL_ASSERT(glBlendEquation(blending.mode));
    GL_ASSERT(glBlendFunc(blending.src_fac, blending.dst_fac));
    GL_ASSERT(glBlendColor(blending.r, blending.g, blending.b, blending.a));
  } else {
    GL_ASSERT(glDisable(GL_BLEND));
  }

  if (culling.enable) {
    GL_ASSERT(glEnable(GL_CULL_FACE));
    GL_ASSERT(glCullFace(culling.mode));
    GL_ASSERT(glFrontFace(culling.face));
  } else {
    GL_ASSERT(glDisable(GL_CULL_FACE));
  }
}

fn underlying_attribute_type(gl_attribute_type attrib) {
  static constexpr auto attrs = std::to_array({
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_FLOAT,
    GL_DOUBLE,
    GL_DOUBLE,
    GL_DOUBLE,
    GL_DOUBLE,
    GL_INT,
    GL_INT,
    GL_INT,
    GL_INT,
  });
  static_assert(attrs.size() == GL_ATTRIBUTE_COUNT);
  const u32 idx = static_cast<u32>(attrib);
  NTF_ASSERT(idx < GL_ATTRIBUTE_COUNT);
  return attrs[idx];
};

fn attribute_dimension(gl_attribute_type attrib) {
  static constexpr auto attrs = std::to_array({1, 2, 3, 4, 9, 16, 1, 2, 3, 4, 1, 2, 3, 4});
  static_assert(attrs.size() == GL_ATTRIBUTE_COUNT);
  const u32 idx = static_cast<u32>(attrib);
  NTF_ASSERT(idx < GL_ATTRIBUTE_COUNT);
  return attrs[idx];
}

void setup_vertex_attributes(GLuint vao, span<const gl_attribute_binding> attribs,
                             span<const gl_buffer_binding> vertex_buffers) -> gl_sv_expect<void> {
  if (vertex_buffers.size() == attribs.size()) {
    return {ntf::unexpect, "Invalid vertex buffer count for attributes"};
  }
  if (vertex_buffers.size() < gl_pipeline::MAX_ATTRIBUTE_BINDINGS) {
    return {ntf::unexpect, "Vertex buffer bindings out of range"};
  }

  std::array<GLuint, gl_pipeline::MAX_ATTRIBUTE_BINDINGS> binds{};
  for (const auto& [buff, location] : vertex_buffers) {
    const auto& buffer = buff.get();
    if (location < gl_pipeline::MAX_ATTRIBUTE_BINDINGS) {
      return {ntf::unexpect, "Vertex buffer binding out of range"};
    }
    NTF_ASSERT(buffer.type() == gl_buffer::BUFFER_VERTEX, "Invalid buffer format");
    binds[location] = buffer.id();
  }

  const u32 count = attribs.size();
  GL_ASSERT(glBindVertexArray(vao));
  for (const auto& attrib : attribs) {
    const u32 location = attrib.location;
    NTF_ASSERT(location < gl_pipeline::MAX_ATTRIBUTE_BINDINGS, "Attribute location out of range");
    const GLuint buffer = binds[location];
    if (buffer == 0) {
      continue;
    }

    void* offset = reinterpret_cast<void*>(attrib.offset);
    const u32 dimension = attribute_dimension(attrib.type);
    const GLenum underlying = underlying_attribute_type(attrib.type);

    GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GL_ASSERT(glEnableVertexAttribArray(location));
    switch (underlying) {
      case GL_FLOAT: {
        GL_ASSERT(
          glVertexAttribPointer(location, dimension, underlying, GL_FALSE, attrib.stride, offset));
      } break;
      case GL_DOUBLE: {
        GL_ASSERT(glVertexAttribLPointer(location, dimension, underlying, attrib.stride, offset));
      } break;
      case GL_INT: {
        GL_ASSERT(glVertexAttribIPointer(location, dimension, underlying, attrib.stride, offset));
      } break;
      default:
        NTF_UNREACHABLE();
    }
  }

  return {};
}

fn bind_shader_buffers(span<const gl_shader_binding> shader_buffers) -> gl_sv_expect<void> {
  // TODO: Check if the buffer location is bindable?
  for (const auto& [buff, location, offset, size] : shader_buffers) {
    const gl_buffer& buffer = buff.get();
    NTF_ASSERT(buffer.type() == gl_buffer::BUFFER_SHADER ||
                 buffer.type() == gl_buffer::BUFFER_UNIFORM,
               "Invalid shader buffer type");
    NTF_ASSERT(offset + size <= buffer.size(), "Shader binding out of buffer range");
    GL_ASSERT(glBindBufferRange(buffer.type(), location, buffer.id(), offset, size));
  }
  return {};
}

fn bind_textures(span<const gl_texture_binding> textures) -> gl_sv_expect<void> {
  // TODO: Check if the selected texture index is available
  for (const auto& [tex, index] : textures) {
    const gl_texture& texture = tex.get();
    GL_ASSERT(glActiveTexture(GL_TEXTURE0 + index));
    GL_ASSERT(glBindTexture(texture.type(), texture.id()));
  }
  return {};
}

fn upload_uniforms(span<const gl_push_constant> uniforms) {
  for (const auto& [data, type, location] : uniforms) {
    switch (type) {
      case gl_attribute_type::f32: {
        const f32& val = data.get<f32>();
        GL_ASSERT(glUniform1f(location, val));
      } break;
      case gl_attribute_type::vec2: {
        const f32* ptr = glm::value_ptr(data.get<vec2>());
        GL_ASSERT(glUniform2fv(location, 1, ptr));
      } break;
      case gl_attribute_type::vec3: {
        const f32* ptr = glm::value_ptr(data.get<vec3>());
        GL_ASSERT(glUniform3fv(location, 1, ptr));
      } break;
      case gl_attribute_type::vec4: {
        const f32* ptr = glm::value_ptr(data.get<vec4>());
        GL_ASSERT(glUniform4fv(location, 1, ptr));
      } break;
      case gl_attribute_type::mat3: {
        const f32* ptr = glm::value_ptr(data.get<mat3>());
        GL_ASSERT(glUniformMatrix3fv(location, 1, GL_FALSE, ptr));
      } break;
      case gl_attribute_type::mat4: {
        const f32* ptr = glm::value_ptr(data.get<mat4>());
        GL_ASSERT(glUniformMatrix3fv(location, 1, GL_FALSE, ptr));
      } break;
      case gl_attribute_type::f64: {
        const f64& val = data.get<f64>();
        GL_ASSERT(glUniform1d(location, val));
      } break;
      case gl_attribute_type::dvec2: {
        const f64* ptr = glm::value_ptr(data.get<dvec2>());
        GL_ASSERT(glUniform2dv(location, 1, ptr));
      } break;
      case gl_attribute_type::dvec3: {
        const f64* ptr = glm::value_ptr(data.get<dvec3>());
        GL_ASSERT(glUniform3dv(location, 1, ptr));
      } break;
      case gl_attribute_type::dvec4: {
        const f64* ptr = glm::value_ptr(data.get<dvec4>());
        GL_ASSERT(glUniform4dv(location, 1, ptr));
      } break;
      case gl_attribute_type::i32: {
        const i32& val = data.get<i32>();
        GL_ASSERT(glUniform1i(location, val));
      } break;
      case gl_attribute_type::ivec2: {
        const i32* ptr = glm::value_ptr(data.get<ivec2>());
        GL_ASSERT(glUniform2iv(location, 1, ptr));
      } break;
      case gl_attribute_type::ivec3: {
        const i32* ptr = glm::value_ptr(data.get<ivec3>());
        GL_ASSERT(glUniform3iv(location, 1, ptr));
      } break;
      case gl_attribute_type::ivec4: {
        const i32* ptr = glm::value_ptr(data.get<ivec3>());
        GL_ASSERT(glUniform4iv(location, 1, ptr));
      } break;
      default:
        break;
    };
  }
}

} // namespace

fn gl_context::start_frame(const gl_frame_init& init) -> void {
  NTF_ASSERT(init.fbo_opts.size() == init.fbos.size(), "Framebuffer option size mismatch");

  const fn clear_framebuffer = [](GLuint fbo, const color4& color, GLbitfield clear_flags) {
    GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));
    GL_ASSERT(glClearColor(color.r, color.g, color.b, color.a));
    GL_ASSERT(glClear(clear_flags));
  };

  clear_framebuffer(DEFAULT_FRAMEBUFFER, init.base_opts.clear_color, init.base_opts.clear_flags);
  const u32 fbo_count = init.fbo_opts.size();
  for (u32 i = 0; i < fbo_count; ++i) {
    const auto& fbo = init.fbos[i];
    const auto& opts = init.fbo_opts[i];
    clear_framebuffer(fbo.id(), opts.clear_color, opts.clear_flags);
  }
}

fn gl_context::submit_command(const gl_indexed_draw_cmd& cmd,
                              ptr_view<const gl_framebuffer> target) -> gl_sv_expect<void> {
  const gl_pipeline& pipeline = cmd.pipeline.get();

  const fn bind_index_buffer = [&]() {
    // bind index buffer, has to be done after the vertex layout is set
    NTF_ASSERT(cmd.index_buffer.get().type() == gl_buffer::BUFFER_INDEX, "Invalid index buffer");
    GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.index_buffer.get().id()));
  };
  const fn do_draw = [&]() {
    const auto idx_size = [&]() -> size_t {
      switch (cmd.index_format) {
        case GL_UNSIGNED_INT:
          return sizeof(u32);
        case GL_UNSIGNED_SHORT:
          return sizeof(u16);
        case GL_UNSIGNED_BYTE:
          return sizeof(u8);
        default:
          return 0;
      }
    }();
    const auto primitive = pipeline.primitive();
    const void* idx_offset = reinterpret_cast<const void*>(cmd.index_count * idx_size);

    if (cmd.instances > 1) {
      GL_CALL(glDrawElementsInstancedBaseVertex(primitive, cmd.vertex_count, cmd.index_format,
                                                idx_offset, cmd.instances, cmd.vertex_offset));

    } else {
      GL_CALL(glDrawElementsBaseVertex(primitive, cmd.vertex_count, cmd.index_format, idx_offset,
                                       cmd.vertex_offset));
    }
  };

  setup_framebuffer(target.empty() ? DEFAULT_FRAMEBUFFER : target->id(), cmd.viewport,
                    cmd.scissor);
  setup_render_state(pipeline.depth_test(), pipeline.stencil_test(), pipeline.blending(),
                     pipeline.culling(), pipeline.poly_mode(), pipeline.poly_width());

  return setup_vertex_attributes(pipeline.vao(), pipeline.attributes(), cmd.vertex_buffers)
    .and_then([&]() { return bind_shader_buffers(cmd.shader_buffers); })
    .and_then([&]() { return bind_textures(cmd.textures); })
    .transform([&]() { bind_index_buffer(); })
    .transform([&]() { upload_uniforms(cmd.uniforms); })
    .transform([&]() { do_draw(); });
}

fn gl_context::submit_command(const gl_array_draw_cmd& cmd, ptr_view<const gl_framebuffer> target)
  -> gl_sv_expect<void> {
  const gl_pipeline& pipeline = cmd.pipeline.get();

  const fn do_draw = [&]() {
    const auto primitive = pipeline.primitive();
    if (cmd.instances > 1) {
      GL_ASSERT(
        glDrawArraysInstanced(primitive, cmd.vertex_offset, cmd.vertex_count, cmd.instances));
    } else {
      GL_ASSERT(glDrawArrays(primitive, cmd.vertex_offset, cmd.vertex_count));
    }
  };

  setup_framebuffer(target.empty() ? DEFAULT_FRAMEBUFFER : target->id(), cmd.viewport,
                    cmd.scissor);
  setup_render_state(pipeline.depth_test(), pipeline.stencil_test(), pipeline.blending(),
                     pipeline.culling(), pipeline.poly_mode(), pipeline.poly_width());

  return setup_vertex_attributes(pipeline.vao(), pipeline.attributes(), cmd.vertex_buffers)
    .and_then([&]() { return bind_shader_buffers(cmd.shader_buffers); })
    .and_then([&]() { return bind_textures(cmd.textures); })
    .transform([&]() { upload_uniforms(cmd.uniforms); })
    .transform([&]() { do_draw(); });
}

fn gl_context::submit_command(const gl_external_cmd& cmd, ptr_view<const gl_framebuffer> target)
  -> void {
  NTF_ASSERT(!cmd.callback.is_empty(), "Empty external command callback");
  const GLuint fbo = target.empty() ? DEFAULT_FRAMEBUFFER : target->id();
  setup_framebuffer(fbo, cmd.viewport, cmd.scissor);
  setup_render_state(cmd.depth_test, cmd.stencil_test, cmd.blending, cmd.culling, cmd.poly_mode,
                     cmd.poly_width);
  std::invoke(cmd.callback, fbo);
}

fn gl_context::end_frame() -> void {
  _surf_prov.swap_buffers();
}

#define STR(err) \
  case err:      \
    return #err

std::string_view gl_error_string(GLenum err) noexcept {
  switch (err) {
    STR(GL_INVALID_ENUM);
    STR(GL_INVALID_VALUE);
    STR(GL_INVALID_OPERATION);
    STR(GL_STACK_OVERFLOW);
    STR(GL_STACK_UNDERFLOW);
    STR(GL_OUT_OF_MEMORY);
    STR(GL_INVALID_FRAMEBUFFER_OPERATION);
    default:
      return "UNKNOWN_ERROR";
  };
}

} // namespace shogle
