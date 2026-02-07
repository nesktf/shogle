#include "./context_private.hpp"
#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/framebuffer.hpp>
#include <shogle/render/gl/texture.hpp>
#include <shogle/render/gl/vertex.hpp>

namespace shogle {

gl_clear_builder::gl_clear_builder() noexcept : _color(), _viewport(), _clear_flags(), _fbos() {}

gl_clear_builder& gl_clear_builder::set_clear_color(const color4& color) {
  return this->set_clear_color(color.r, color.g, color.b, color.a);
}

gl_clear_builder& gl_clear_builder::set_clear_color(f32 r, f32 g, f32 b, f32 a) {
  _color.r = r;
  _color.g = g;
  _color.b = b;
  _color.a = a;
  return *this;
}

gl_clear_builder& gl_clear_builder::set_clear_flag(gl_clear_opts::clear_flag clear_flag) {
  _clear_flags |= (gldefs::GLenum)clear_flag;
  return *this;
}

gl_clear_builder& gl_clear_builder::add_framebuffer(const gl_framebuffer& fbo,
                                                    const rectangle_pos<u32>& viewport,
                                                    gldefs::GLenum clear_flags,
                                                    const color4& clear_color) {
  _fbos.emplace_back(clear_color, viewport, clear_flags, fbo.id());
  return *this;
}

gl_clear_builder& gl_clear_builder::add_framebuffer(const gl_framebuffer& fbo,
                                                    const rectangle_pos<u32>& viewport,
                                                    gldefs::GLenum clear_flags, f32 r, f32 g,
                                                    f32 b, f32 a) {
  const color4 clear_color(r, g, b, a);
  return this->add_framebuffer(fbo, viewport, clear_flags, clear_color);
}

void gl_clear_builder::reset() {
  _color.r = 0.f;
  _color.g = 0.f;
  _color.b = 0.f;
  _color.a = 0.f;
  _clear_flags = 0;
  _fbos.clear();
}

gl_clear_opts gl_clear_builder::build() const {
  return {
    .clear_color = _color,
    .viewport = _viewport,
    .clear_flags = _clear_flags,
    .fbos = {_fbos.data(), _fbos.size()},
  };
}

gl_command_builder::gl_command_builder() noexcept :
    _vertex_layout(), _pipeline(), _vertex_binds(), _shader_binds(), _texture_binds(), _uniforms(),
    _index(), _viewport(), _scissor(), _instances(1u) {}

void gl_command_builder::reset() {
  _vertex_layout = nullptr;
  _pipeline = nullptr;
  _vertex_binds.clear();
  _shader_binds.clear();
  _texture_binds.clear();
  _uniforms.clear();
  if (_index.has_value()) {
    _index.reset();
  }
  _viewport.x = 0;
  _viewport.y = 0;
  _viewport.width = 0;
  _viewport.height = 0;
  if (_scissor.has_value()) {
    _scissor.reset();
  }
  _instances = 1;
}

gl_command_builder& gl_command_builder::set_vertex_layout(const gl_vertex_layout& layout) {
  _vertex_layout = layout;
  return *this;
}

gl_command_builder& gl_command_builder::set_pipeline(const gl_graphics_pipeline& pipeline) {
  _pipeline = pipeline;
  return *this;
}

gl_command_builder& gl_command_builder::set_viewport(const rectangle_pos<u32>& viewport) {
  _viewport = viewport;
  return *this;
}

gl_command_builder& gl_command_builder::set_viewport(u32 x, u32 y, u32 width, u32 height) {
  const rectangle_pos<u32> viewport(x, y, width, height);
  return set_viewport(viewport);
}

gl_command_builder& gl_command_builder::set_scissor(const rectangle_pos<u32>& scissor) {
  if (_scissor.has_value()) {
    *_scissor = scissor;
  } else {
    _scissor.emplace(scissor);
  }
  return *this;
}

gl_command_builder& gl_command_builder::set_scissor(u32 x, u32 y, u32 width, u32 height) {
  const rectangle_pos<u32> scissor(x, y, width, height);
  return set_scissor(scissor);
}

gl_command_builder& gl_command_builder::set_instances(u32 instances) {
  _instances = instances;
  return *this;
}

gl_command_builder& gl_command_builder::set_vertex_offset(size_t offset) {
  _vertex_offset = offset;
  return *this;
}

gl_command_builder& gl_command_builder::set_vertex_count(u32 count) {
  _vertex_count = count;
  return *this;
}

gl_command_builder& gl_command_builder::set_index_buffer(const gl_buffer& buffer,
                                                         gl_draw_command::index_format format,
                                                         u32 index_count) {
  NTF_ASSERT(buffer.type() == gl_buffer::TYPE_INDEX, "Binding non index buffer for indices");
  if (_index.has_value()) {
    _index->buffer = buffer.id();
    _index->format = format;
    _index->index_count = index_count;
  } else {
    _index.emplace(buffer.id(), format, index_count);
  }
  return *this;
}

gl_command_builder& gl_command_builder::add_vertex_buffer(u32 location, const gl_buffer& buffer) {
  NTF_ASSERT(buffer.type() == gl_buffer::TYPE_VERTEX, "Binding non vertex buffer for vertices");
  _vertex_binds.emplace_back(buffer.id(), location);
  return *this;
}

gl_command_builder& gl_command_builder::add_shader_buffer(u32 location, const gl_buffer& buffer,
                                                          size_t size, size_t offset) {
  NTF_ASSERT(buffer.type() == gl_buffer::TYPE_SHADER || buffer.type() == gl_buffer::TYPE_UNIFORM,
             "Binding non shader buffer to shader");
  _shader_binds.emplace_back(buffer.id(), (gldefs::GLenum)buffer.type(), size, offset, location);
  return *this;
}

gl_command_builder& gl_command_builder::add_texture(u32 index, const gl_texture& texture) {
  _texture_binds.emplace_back(texture.id(), index);
  return *this;
}

gl_draw_command gl_command_builder::build() const {
  NTF_ASSERT(!_vertex_layout.empty(), "No vertex layout provided in builder");
  NTF_ASSERT(!_pipeline.empty(), "No pipeline provided in builder");
  return {
    .vertex_layout = *_vertex_layout,
    .pipeline = *_pipeline,
    .vertex_bindings = {_vertex_binds.data(), _vertex_binds.size()},
    .shader_bindings = {_shader_binds.data(), _shader_binds.size()},
    .texture_bindings = {_texture_binds.data(), _texture_binds.size()},
    .uniforms = {_uniforms.data(), _uniforms.size()},
    .index_bind = _index,
    .viewport = _viewport,
    .scissor = _scissor.has_value() ? *_scissor : _viewport,
    .vertex_offset = _vertex_offset,
    .vertex_count = _vertex_count,
    .instances = std::max(_instances, 1u),
  };
}

gl_external_command_builder::gl_external_command_builder() noexcept :
    _callback(), _stencil(::shogle::gl_stencil_test_props::make_default(false)),
    _depth(::shogle::gl_depth_test_props::make_default(false)),
    _blending(::shogle::gl_blending_props::make_default(false)),
    _culling(::shogle::gl_culling_props::make_default(false)),
    _primitive(gl_graphics_pipeline::PRIMITIVE_TRIANGLES),
    _poly_mode(gl_graphics_pipeline::POLY_MODE_FILL), _poly_width(1.f), _viewport(), _scissor() {}

gl_external_command_builder&
gl_external_command_builder::set_callback(gl_external_command::callback_type callback) {
  _callback = callback;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_depth_test(const gl_depth_test_props& depth) {
  _depth = depth;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_stencil_test(const gl_stencil_test_props& stencil) {
  _stencil = stencil;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_blending(const gl_blending_props& blending) {
  _blending = blending;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_culling(const gl_culling_props& culling) {
  _culling = culling;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_primitive(gl_graphics_pipeline::primitive_mode primitive) {
  _primitive = primitive;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_poly_mode(gl_graphics_pipeline::polygon_mode poly_mode) {
  _poly_mode = poly_mode;
  return *this;
}

gl_external_command_builder&
gl_external_command_builder::set_viewport(const rectangle_pos<u32>& viewport) {
  _viewport = viewport;
  return *this;
}

gl_external_command_builder& gl_external_command_builder::set_viewport(u32 x, u32 y, u32 width,
                                                                       u32 height) {
  const rectangle_pos<u32> viewport(x, y, width, height);
  return set_viewport(viewport);
}

gl_external_command_builder&
gl_external_command_builder::set_scissor(const rectangle_pos<u32>& scissor) {
  return this->set_scissor(scissor.x, scissor.y, scissor.width, scissor.height);
}

gl_external_command_builder& gl_external_command_builder::set_scissor(u32 x, u32 y, u32 width,
                                                                      u32 height) {
  if (_scissor.has_value()) {
    _scissor->x = x;
    _scissor->y = y;
    _scissor->width = width;
    _scissor->height = height;
  } else {
    _scissor.emplace(x, y, width, height);
  }
  return *this;
}

void gl_external_command_builder::reset() {
  _callback = nullptr;
  _stencil = gl_stencil_test_props::make_default(false);
  _depth = gl_depth_test_props::make_default(false);
  _blending = gl_blending_props::make_default(false);
  _culling = gl_culling_props::make_default(false);
  _primitive = gl_graphics_pipeline::PRIMITIVE_TRIANGLES;
  _poly_mode = gl_graphics_pipeline::POLY_MODE_FILL;
  _poly_width = 1.f;
  _viewport.x = 0;
  _viewport.y = 0;
  _viewport.width = 0;
  _viewport.height = 0;
  _scissor.reset();
}

gl_external_command gl_external_command_builder::build() const {
  NTF_ASSERT(!_callback.is_empty(), "Callback not bound to external command");
  return {
    .callback = _callback,
    .depth_test = _depth,
    .stencil_test = _stencil,
    .blending = _blending,
    .culling = _culling,
    .primitive = _primitive,
    .poly_mode = _poly_mode,
    .poly_width = _poly_width,
    .viewport = _viewport,
    .scissor = this->_scissor.has_value() ? *this->_scissor : this->_viewport,
  };
}

namespace {

APIENTRY void debug_callback(GLenum src, GLenum type, GLenum id, GLenum severity, GLsizei,
                             const char* message, const void* user) {
  NTF_UNUSED(user);

  std::string_view severity_msg = [severity]() {
    switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH:
        return "HIGH";
      case GL_DEBUG_SEVERITY_MEDIUM:
        return "MEDIUM";
      case GL_DEBUG_SEVERITY_LOW:
        return "LOW";
      case GL_DEBUG_SEVERITY_NOTIFICATION:
        return "NOTIFICATION";

      default:
        break;
    }
    return "UNKNOWN_SEVERITY";
  }();

  std::string_view type_msg = [type]() {
    switch (type) {
      case GL_DEBUG_TYPE_ERROR:
        return "ERROR";
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        return "DEPRECATED_BEHAVIOR";
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        return "UNDEFINED_BEHAVIOR";
      case GL_DEBUG_TYPE_PORTABILITY:
        return "PORTABILITY";
      case GL_DEBUG_TYPE_PERFORMANCE:
        return "PERFORMANCE";
      case GL_DEBUG_TYPE_MARKER:
        return "MARKER";
      case GL_DEBUG_TYPE_PUSH_GROUP:
        return "PUSH_GROUP";
      case GL_DEBUG_TYPE_POP_GROUP:
        return "POP_GROUP";
      case GL_DEBUG_TYPE_OTHER:
        return "OTHER";

      default:
        break;
    };
    return "UNKNOWN_TYPE";
  }();

  std::string_view src_msg = [src]() {
    switch (src) {
      case GL_DEBUG_SOURCE_API:
        return "API";
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        return "WINDOW_SYSTEM";
      case GL_DEBUG_SOURCE_SHADER_COMPILER:
        return "SHADER_COMPILER";
      case GL_DEBUG_SOURCE_THIRD_PARTY:
        return "THIRD_PARTY";
      case GL_DEBUG_SOURCE_APPLICATION:
        return "APPLICATION";
      case GL_DEBUG_SOURCE_OTHER:
        return "OTHER";

      default:
        break;
    };
    return "UNKNOWN_SOURCE";
  }();

  if (type == GL_DEBUG_TYPE_ERROR) {
    SHOGLE_GL_LOG(error, "Debug ({})({})({})({}) {}", severity_msg, type_msg, src_msg, id,
                  message);
  } else {
    SHOGLE_GL_LOG(verbose, "Debug ({})({})({})({}) {}", severity_msg, type_msg, src_msg, id,
                  message);
  }
}

} // namespace

sv_expect<gl_context> gl_context::create(gl_surface_provider& surf_prov) noexcept {
  static constexpr size_t initial_arena_pages = 16;
  const size_t initial_arena_size = initial_arena_pages * ntf::mem::system_page_size();

  try {
    auto arena = scratch_arena::with_initial_size(initial_arena_size);
    if (!arena) {
      return {ntf::unexpect, "Failed to allocate scratch arena"};
    }

    context_data ctx(ntf::mem::default_pool::instance().construct<gl_private>(std::move(*arena)));
    ctx->surf_prov = static_cast<void*>(&surf_prov);

#if defined(SHOGLE_USE_SYSTEM_GL) && SHOGLE_USE_SYSTEM_GL
    ctx->version_string = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!ctx->version_string) {
      return {ntf::unexpect, "Failed to retrieve OpenGL version"};
    }
    shogle_gl_get_version(ctx->version_string, &ctx->ver);
    ctx->vendor_string = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    ctx->renderer_string = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    glDebugMessageCallback((GLDEBUGPROC)debug_callback, ctx.get());
#else
    const auto proc = surf_prov.gl_proc_loader();
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
    ctx->funcs.glDebugMessageCallback((GLDEBUGPROC)debug_callback, ctx.get());
#endif
    NTF_ASSERT(ctx->version_string);
    NTF_ASSERT(ctx->vendor_string);
    NTF_ASSERT(ctx->renderer_string);
    SHOGLE_GL_LOG(debug, "OpenGL context created [{} - {} - {}]", ctx->version_string,
                  ctx->vendor_string, ctx->renderer_string);
    return {ntf::in_place, create_t{}, std::move(ctx)};
  } catch (...) {
    return {ntf::unexpect, "Failed to allocate OpenGL context"};
  }
}

void gl_context::context_deleter::operator()(gl_private* ptr) noexcept {
  ntf::alloc_destroy(ptr);
}

gl_context::gl_context(create_t, context_data&& ctx) noexcept : _ctx(std::move(ctx)) {}

gl_context::gl_context(gl_surface_provider& surf_prov) :
    gl_context(::shogle::gl_context::create(surf_prov).value()) {}

gl_private& impl::gl_get_private(gl_context& gl) {
  NTF_ASSERT(gl._ctx, "gl_context use after move");
  return *gl._ctx;
}

scratch_arena& impl::gl_get_scratch_arena(gl_context& gl) {
  return impl::gl_get_private(gl).arena;
}

gl_surface_provider& gl_context::provider() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  return *static_cast<gl_surface_provider*>(_ctx->surf_prov);
}

gldefs::GLenum gl_context::get_error() const {
  NTF_ASSERT(_ctx, "gl_context use after move");
  gldefs::GLenum out = 0;
  gldefs::GLenum err;
  const auto& funcs = _ctx->funcs;
  while ((err = funcs.glGetError()) != GL_NO_ERROR) {
    out = err;
  }
  return out;
}

gl_context::gl_version gl_context::version() const {
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

namespace {

constexpr GLuint DEFAULT_FRAMEBUFFER = 0;

void setup_framebuffer(gl_context& gl, GLuint fbo, const rectangle_pos<u32>& viewport,
                       const rectangle_pos<u32>& scissor) {
  GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));
  GL_ASSERT(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
  GL_ASSERT(glEnable(GL_SCISSOR_TEST));
  GL_ASSERT(glScissor(scissor.x, scissor.y, scissor.width, scissor.height));
}

void setup_render_state(gl_context& gl, const gl_depth_test_props& depth_test,
                        const gl_stencil_test_props& stencil_test,
                        const gl_blending_props& blending, const gl_culling_props& culling,
                        GLenum poly_mode, f32 poly_width) {
  GL_ASSERT(glPolygonMode(GL_FRONT_AND_BACK, poly_mode));
  if (poly_mode == GL_LINE) {
    GL_ASSERT(glLineWidth(poly_width));
  } else {
    GL_ASSERT(glPointSize(poly_width));
  }

  if (depth_test.enable) {
    GL_ASSERT(glEnable(GL_DEPTH_TEST));
    GL_ASSERT(glDepthFunc(depth_test.test));
    GL_ASSERT(glDepthRange(depth_test.near, depth_test.far));
    GL_ASSERT(glDepthMask(depth_test.mask));
  } else {
    GL_ASSERT(glDisable(GL_DEPTH_TEST));
  }

  if (stencil_test.enable) {
    GL_ASSERT(glEnable(GL_STENCIL_TEST));
    GL_ASSERT(glStencilFunc(stencil_test.test, stencil_test.test_ref, stencil_test.test_mask));
    GL_ASSERT(
      glStencilOp(stencil_test.on_stencil_fail, stencil_test.on_depth_fail, stencil_test.on_pass));
    GL_ASSERT(glStencilMask(stencil_test.stencil_mask));
  } else {
    GL_ASSERT(glDisable(GL_STENCIL_TEST));
  }

  if (blending.enable) {
    GL_ASSERT(glEnable(GL_BLEND));
    GL_ASSERT(glBlendEquation(blending.mode));
    GL_ASSERT(glBlendFuncSeparate(blending.src_color, blending.dst_color, blending.src_alpha,
                                  blending.dst_alpha));
    GL_ASSERT(
      glBlendColor(blending.color.r, blending.color.g, blending.color.b, blending.color.a));
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

gldefs::GLenum underlying_attribute_type(attribute_type attrib) {
  static constexpr auto attrs = std::to_array({
    GL_FLOAT,        // f32
    GL_FLOAT,        // vec2
    GL_FLOAT,        // vec3
    GL_FLOAT,        // vec4
    GL_FLOAT,        // mat3
    GL_FLOAT,        // mat4
    GL_DOUBLE,       // f64
    GL_DOUBLE,       // dvec2
    GL_DOUBLE,       // dvec3
    GL_DOUBLE,       // dvec4
    GL_INT,          // i32
    GL_INT,          // ivec2
    GL_INT,          // ivec3
    GL_INT,          // ivec4
    GL_UNSIGNED_INT, // u32
    GL_UNSIGNED_INT, // uvec2
    GL_UNSIGNED_INT, // uvec3
    GL_UNSIGNED_INT, // uvec4
  });
  const u32 idx = static_cast<u32>(attrib);
  return idx < attrs.size() ? attrs[idx] : 0;
};

u32 attribute_dimension(attribute_type attrib) {
  return ::shogle::meta::attribute_dim(attrib);
}

auto setup_vertex_attributes(gl_context& gl, const gl_vertex_layout& layout,
                             span<const gl_buffer_binding> vertex_buffers) -> gl_sv_expect<void> {
  const auto attribs = layout.attributes();
  if (vertex_buffers.size() == attribs.size()) {
    return {ntf::unexpect, "Invalid vertex buffer count for attributes"};
  }
  if (vertex_buffers.size() < gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS) {
    return {ntf::unexpect, "Vertex buffer bindings out of range"};
  }

  std::array<GLuint, gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS> binds{};
  for (const auto& [buff, location] : vertex_buffers) {
    const auto& buffer = buff.get();
    if (location >= binds.size()) {
      return {ntf::unexpect, "Vertex buffer binding out of range"};
    }
    if (buffer.type() != gl_buffer::BUFFER_VERTEX) {
      SHOGLE_GL_LOG(warning,
                    "Binding non vertex buffer with id {} on vertex attribute {} of VAO {}",
                    buffer.id(), location, layout.vao());
    }
    binds[location] = buffer.id();
  }

  GL_ASSERT(glBindVertexArray(layout.vao()));
  for (const auto& attrib : attribs) {
    const u32 location = attrib.location;
    NTF_ASSERT(location < gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS,
               "Attribute location out of range");
    const GLuint buffer = binds[location];
    if (buffer == 0) {
      continue;
    }

    void* offset = reinterpret_cast<void*>(attrib.offset);
    const u32 dimension = attribute_dimension(attrib.type);
    const auto underlying = underlying_attribute_type(attrib.type);

    GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GL_ASSERT(glEnableVertexAttribArray(location));
    switch (underlying) {
      case GL_FLOAT: {
        GL_ASSERT(glVertexAttribPointer(location, dimension, underlying, GL_FALSE, layout.stride(),
                                        offset));
      } break;
      case GL_DOUBLE: {
        GL_ASSERT(
          glVertexAttribLPointer(location, dimension, underlying, layout.stride(), offset));
      } break;
      case GL_INT: {
        GL_ASSERT(
          glVertexAttribIPointer(location, dimension, underlying, layout.stride(), offset));
      } break;
      default:
        NTF_UNREACHABLE();
    }
  }

  return {};
}

gl_sv_expect<void> bind_shader_buffers(gl_context& gl,
                                       span<const gl_shader_binding> shader_buffers) {
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

gl_sv_expect<void> bind_textures(gl_context& gl, span<const gl_texture_binding> textures) {
  // TODO: Check if the selected texture index is available
  for (const auto& [tex, index] : textures) {
    const gl_texture& texture = tex.get();
    GL_ASSERT(glActiveTexture(GL_TEXTURE0 + index));
    GL_ASSERT(glBindTexture(texture.type(), texture.id()));
  }
  return {};
}

void upload_uniforms(gl_context& gl, span<const gl_push_uniform> uniforms) {
  for (const auto& [data, type, location] : uniforms) {
    switch (type) {
      case attribute_type::f32: {
        const f32& val = data.get<f32>();
        GL_ASSERT(glUniform1f(location, val));
      } break;
      case attribute_type::vec2: {
        const f32* ptr = ::shogle::math::vec_ptr(data.get<math::vec2>());
        GL_ASSERT(glUniform2fv(location, 1, ptr));
      } break;
      case attribute_type::vec3: {
        const f32* ptr = ::shogle::math::vec_ptr(data.get<math::vec3>());
        GL_ASSERT(glUniform3fv(location, 1, ptr));
      } break;
      case attribute_type::vec4: {
        const f32* ptr = ::shogle::math::vec_ptr(data.get<math::vec4>());
        GL_ASSERT(glUniform4fv(location, 1, ptr));
      } break;
      case attribute_type::mat3: {
        const f32* ptr = ::shogle::math::vec_ptr(data.get<math::mat3>());
        GL_ASSERT(glUniformMatrix3fv(location, 1, GL_FALSE, ptr));
      } break;
      case attribute_type::mat4: {
        const f32* ptr = ::shogle::math::vec_ptr(data.get<math::mat4>());
        GL_ASSERT(glUniformMatrix4fv(location, 1, GL_FALSE, ptr));
      } break;
      case attribute_type::f64: {
        const f64& val = data.get<f64>();
        GL_ASSERT(glUniform1d(location, val));
      } break;
      case attribute_type::dvec2: {
        const f64* ptr = ::shogle::math::vec_ptr(data.get<math::dvec2>());
        GL_ASSERT(glUniform2dv(location, 1, ptr));
      } break;
      case attribute_type::dvec3: {
        const f64* ptr = ::shogle::math::vec_ptr(data.get<math::dvec3>());
        GL_ASSERT(glUniform3dv(location, 1, ptr));
      } break;
      case attribute_type::dvec4: {
        const f64* ptr = ::shogle::math::vec_ptr(data.get<math::dvec4>());
        GL_ASSERT(glUniform4dv(location, 1, ptr));
      } break;
      case attribute_type::i32: {
        const i32& val = data.get<i32>();
        GL_ASSERT(glUniform1i(location, val));
      } break;
      case attribute_type::ivec2: {
        const i32* ptr = ::shogle::math::vec_ptr(data.get<math::ivec2>());
        GL_ASSERT(glUniform2iv(location, 1, ptr));
      } break;
      case attribute_type::ivec3: {
        const i32* ptr = ::shogle::math::vec_ptr(data.get<math::ivec3>());
        GL_ASSERT(glUniform3iv(location, 1, ptr));
      } break;
      case attribute_type::ivec4: {
        const i32* ptr = ::shogle::math::vec_ptr(data.get<math::ivec3>());
        GL_ASSERT(glUniform4iv(location, 1, ptr));
      } break;
      case attribute_type::u32: {
        const u32& val = data.get<u32>();
        GL_ASSERT(glUniform1ui(location, val));
      } break;
      case attribute_type::uvec2: {
        const u32* ptr = ::shogle::math::vec_ptr(data.get<math::uvec2>());
        GL_ASSERT(glUniform2uiv(location, 1, ptr));
      } break;
      case attribute_type::uvec3: {
        const u32* ptr = ::shogle::math::vec_ptr(data.get<math::uvec3>());
        GL_ASSERT(glUniform3uiv(location, 1, ptr));
      } break;
      case attribute_type::uvec4: {
        const u32* ptr = ::shogle::math::vec_ptr(data.get<math::uvec3>());
        GL_ASSERT(glUniform4uiv(location, 1, ptr));
      } break;
    };
  }
}

} // namespace

void gl_context::start_frame(const gl_frame_initializer& init) {
  auto& gl = *this;

  const auto clear_framebuffer = [&](GLuint fbo, const color4& color, GLbitfield clear_flags) {
    GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));
    GL_ASSERT(glClearColor(color.r, color.g, color.b, color.a));
    GL_ASSERT(glClear(clear_flags));
  };

  clear_framebuffer(DEFAULT_FRAMEBUFFER, init.clear_opt.color, init.clear_opt.clear_flags);
  for (const auto& [fbo, clear_opt] : init.fbos) {
    clear_framebuffer(fbo->id(), clear_opt.color, clear_opt.clear_flags);
  }
}

gl_sv_expect<void> gl_context::submit_indexed_draw_command(const gl_indexed_cmd& cmd,
                                                           ptr_view<const gl_framebuffer> target) {
  auto& gl = *this;
  const gl_graphics_pipeline& pipeline = *cmd.pipeline;

  const auto bind_index_buffer = [&]() {
    // bind index buffer, has to be done after the vertex layout is set
    NTF_ASSERT(cmd.index_buffer.get().type() == gl_buffer::BUFFER_INDEX, "Invalid index buffer");
    GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.index_buffer.get().id()));
  };
  const auto do_draw = [&]() {
    const auto idx_size = [&]() -> size_t {
      switch (cmd.format) {
        case gl_indexed_cmd::INDEX_FORMAT_U32:
          return sizeof(u32);
        case gl_indexed_cmd::INDEX_FORMAT_U16:
          return sizeof(u16);
        case gl_indexed_cmd::INDEX_FORMAT_U8:
          return sizeof(u8);
        case gl_indexed_cmd::INDEX_FORMAT_I32:
          return sizeof(i32);
        case gl_indexed_cmd::INDEX_FORMAT_I16:
          return sizeof(i16);
        case gl_indexed_cmd::INDEX_FORMAT_I8:
          return sizeof(i8);
        default:
          return 0;
      }
    }();
    const auto primitive = pipeline.primitive();
    const void* idx_offset = reinterpret_cast<const void*>(cmd.index_count * idx_size);

    if (cmd.instances > 1) {
      GL_CALL(glDrawElementsInstancedBaseVertex(primitive, cmd.vertex_count, cmd.format,
                                                idx_offset, cmd.instances, cmd.vertex_offset));

    } else {
      GL_CALL(glDrawElementsBaseVertex(primitive, cmd.vertex_count, cmd.format, idx_offset,
                                       cmd.vertex_offset));
    }
  };

  setup_framebuffer(gl, target.empty() ? DEFAULT_FRAMEBUFFER : target->id(), cmd.viewport,
                    cmd.scissor);
  setup_render_state(gl, pipeline.depth_test(), pipeline.stencil_test(), pipeline.blending(),
                     pipeline.culling(), pipeline.poly_mode(), pipeline.poly_width());

  return setup_vertex_attributes(gl, cmd.vertex_layout, cmd.vertex_buffers)
    .and_then([&]() { return bind_shader_buffers(gl, cmd.shader_buffers); })
    .and_then([&]() { return bind_textures(gl, cmd.textures); })
    .transform([&]() { bind_index_buffer(); })
    .transform([&]() { upload_uniforms(gl, cmd.uniforms); })
    .transform([&]() { do_draw(); });
}

gl_sv_expect<void> gl_context::submit_draw_command(const gl_array_cmd& cmd,
                                                   ptr_view<const gl_framebuffer> target) {
  auto& gl = *this;
  const gl_graphics_pipeline& pipeline = *cmd.pipeline;

  const auto do_draw = [&]() {
    const auto primitive = pipeline.primitive();
    if (cmd.instances > 1) {
      GL_ASSERT(
        glDrawArraysInstanced(primitive, cmd.vertex_offset, cmd.vertex_count, cmd.instances));
    } else {
      GL_ASSERT(glDrawArrays(primitive, cmd.vertex_offset, cmd.vertex_count));
    }
  };

  setup_framebuffer(gl, target.empty() ? DEFAULT_FRAMEBUFFER : target->id(), cmd.viewport,
                    cmd.scissor);
  setup_render_state(gl, pipeline.depth_test(), pipeline.stencil_test(), pipeline.blending(),
                     pipeline.culling(), pipeline.poly_mode(), pipeline.poly_width());

  return setup_vertex_attributes(gl, cmd.vertex_layout, cmd.vertex_buffers)
    .and_then([&]() { return bind_shader_buffers(gl, cmd.shader_buffers); })
    .and_then([&]() { return bind_textures(gl, cmd.textures); })
    .transform([&]() { upload_uniforms(gl, cmd.uniforms); })
    .transform([&]() { do_draw(); });
}

void gl_context::submit_external_command(const gl_external_cmd& cmd,
                                         ptr_view<const gl_framebuffer> target) {
  NTF_ASSERT(!cmd.callback.is_empty(), "Empty external command callback");
  const GLuint fbo = target.empty() ? DEFAULT_FRAMEBUFFER : target->id();
  setup_framebuffer(*this, fbo, cmd.viewport, cmd.scissor);
  setup_render_state(*this, cmd.depth_test, cmd.stencil_test, cmd.blending, cmd.culling,
                     cmd.poly_mode, cmd.poly_width);
  std::invoke(cmd.callback, *this, fbo);
}

void gl_context::end_frame() {
  // No-op
}

std::string_view gl_error_string(GLenum err) noexcept {
#define STR(err) \
  case err:      \
    return #err
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
#undef STR
}

} // namespace shogle
