#include "./context_private.hpp"
#include <shogle/render/gl/buffer.hpp>
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/framebuffer.hpp>
#include <shogle/render/gl/texture.hpp>
#include <shogle/render/gl/vertex.hpp>

namespace shogle {

gl_clear_builder::gl_clear_builder() noexcept : _color(), _viewport(), _clear_flags(), _fbos() {}

gl_clear_builder& gl_clear_builder::set_viewport(u32 x, u32 y, u32 width, u32 height) {
  const rectangle_pos<u32> viewport(x, y, width, height);
  return set_viewport(viewport);
}

gl_clear_builder& gl_clear_builder::set_viewport(const rectangle_pos<u32>& viewport) {
  if (_viewport.has_value()) {
    *_viewport = viewport;
  } else {
    _viewport.emplace(viewport);
  }
  return *this;
}

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
  if (_viewport.has_value()) {
    _viewport.reset();
  }
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
    _index(), _viewport(), _scissor(), _vertex_offset(), _draw_count(), _instances(1u) {}

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
  if (_viewport.has_value()) {
    _viewport.reset();
  }
  if (_scissor.has_value()) {
    _scissor.reset();
  }
  _vertex_offset = 0;
  _draw_count = 0;
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
  if (_viewport.has_value()) {
    *_viewport = viewport;
  } else {
    _viewport.emplace(viewport);
  }
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

gl_command_builder& gl_command_builder::set_draw_count(u32 count) {
  _draw_count = count;
  return *this;
}

gl_command_builder& gl_command_builder::set_index_buffer(const gl_buffer& buffer,
                                                         gl_draw_command::index_format format,
                                                         size_t index_offset) {
  SHOGLE_ASSERT(buffer.type() == gl_buffer::TYPE_INDEX, "Binding non index buffer for indices");
  if (_index.has_value()) {
    _index->buffer = buffer.id();
    _index->format = format;
    _index->index_offset = index_offset;
  } else {
    _index.emplace(buffer.id(), format, index_offset);
  }
  return *this;
}

gl_command_builder& gl_command_builder::add_vertex_buffer(const gl_buffer& buffer, u32 location) {
  SHOGLE_ASSERT(buffer.type() == gl_buffer::TYPE_VERTEX, "Binding non vertex buffer for vertices");
  _vertex_binds.emplace_back(buffer.id(), location);
  return *this;
}

gl_command_builder& gl_command_builder::add_shader_buffer(u32 location, const gl_buffer& buffer,
                                                          size_t size, size_t offset) {
  SHOGLE_ASSERT(buffer.type() == gl_buffer::TYPE_SHADER ||
                  buffer.type() == gl_buffer::TYPE_UNIFORM,
                "Binding non shader buffer to shader");
  SHOGLE_ASSERT(offset + size <= buffer.size(), "Shader binding out of buffer range");
  _shader_binds.emplace_back(buffer.id(), (gldefs::GLenum)buffer.type(), size, offset, location);
  return *this;
}

gl_command_builder& gl_command_builder::add_texture(const gl_texture& texture, u32 index) {
  _texture_binds.emplace_back(texture.id(), texture.type(), index);
  return *this;
}

gl_draw_command gl_command_builder::build() const {
  SHOGLE_ASSERT(!_vertex_layout.empty(), "No vertex layout provided in builder");
  SHOGLE_ASSERT(!_pipeline.empty(), "No pipeline provided in builder");
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
    .draw_count = _draw_count,
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
	if (_callback.has_value()) {
		*_callback = callback;
	} else {
		_callback.emplace(callback);
          }
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
	_callback.reset();
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
  SHOGLE_ASSERT(_callback.has_value(), "Callback not bound to external command");
  return {
    .callback = *_callback,
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

APIENTRY void debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei,
                             const char* message, const void* user) {
  SHOGLE_UNUSED(user);

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

sv_expect<gl_context> gl_context::create(const gl_surface_provider& surf_prov) noexcept {
  static constexpr size_t initial_arena_pages = 16;
  const size_t initial_arena_size = initial_arena_pages * mem::system_page_size();

  try {
    auto arena = mem::scratch_arena::with_initial_size(initial_arena_size);
    if (!arena) {
      return {unexpect, "Failed to allocate scratch arena"};
    }

    context_data ctx(new gl_private(std::move(*arena), surf_prov));

#if defined(SHOGLE_USE_SYSTEM_GL) && SHOGLE_USE_SYSTEM_GL
    ctx->version_string = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (!ctx->version_string) {
      return {unexpect, "Failed to retrieve OpenGL version"};
    }
    shogle_gl_get_version(ctx->version_string, &ctx->ver);
    ctx->vendor_string = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    ctx->renderer_string = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    glDebugMessageCallback((GLDEBUGPROC)debug_callback, ctx.get());
#else
    const auto proc = ctx->surf_prov.get_proc_func();
    auto err = shogle_gl_load_funcs(ctx->surf_prov.get_ptr(), (PFN_shogle_glGetProcAddress)proc,
                                    &ctx->funcs, &ctx->ver);
    if (err) {
      static constexpr auto errors = std::to_array<const char*>(
        {"Failed to load OpenGL functions", "Failed to load OpenGL", "Invalid OpenGL version"});
      return {unexpect, errors[static_cast<u32>(err) - 1]};
    }
    ctx->version_string = reinterpret_cast<const char*>(ctx->funcs.glGetString(GL_VERSION));
    ctx->vendor_string = reinterpret_cast<const char*>(ctx->funcs.glGetString(GL_VENDOR));
    ctx->renderer_string = reinterpret_cast<const char*>(ctx->funcs.glGetString(GL_RENDERER));
    ctx->funcs.glDebugMessageCallback((GLDEBUGPROC)debug_callback, ctx.get());
#endif
    SHOGLE_ASSERT(ctx->version_string);
    SHOGLE_ASSERT(ctx->vendor_string);
    SHOGLE_ASSERT(ctx->renderer_string);
    SHOGLE_GL_LOG(debug, "OpenGL context created (ptr: {})", fmt::ptr(ctx.get()));
    SHOGLE_GL_LOG(debug, "{}, {}, {}", ctx->version_string, ctx->vendor_string,
                  ctx->renderer_string);
    return {in_place, create_t{}, std::move(ctx)};
  } catch (...) {
    return {unexpect, "Failed to allocate OpenGL context"};
  }
}

void gl_context::context_deleter::operator()(gl_private* ptr) noexcept {
  SHOGLE_GL_LOG(debug, "OpenGL context destroyed (ptr: {})", fmt::ptr(ptr));
  delete ptr;
}

gl_context::gl_context(create_t, context_data&& ctx) noexcept : _ctx(std::move(ctx)) {}

gl_context::gl_context(const gl_surface_provider& surf_prov) :
    gl_context(::shogle::gl_context::create(surf_prov).value()) {}

gl_private& impl::gl_get_private(gl_context& gl) {
  SHOGLE_ASSERT(gl._ctx, "gl_context use after free");
  return *gl._ctx;
}

mem::scratch_arena& impl::gl_get_scratch_arena(gl_context& gl) {
  return impl::gl_get_private(gl).arena;
}

gl_surface_provider gl_context::provider() const {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  return _ctx->surf_prov;
}

gldefs::GLenum gl_context::get_error() const {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  gldefs::GLenum out = 0;
  gldefs::GLenum err;
  const auto& funcs = _ctx->funcs;
  while ((err = funcs.glGetError()) != GL_NO_ERROR) {
    out = err;
  }
  return out;
}

gl_context::gl_version gl_context::version() const {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  return gl_version{.major = static_cast<u32>(_ctx->ver.maj),
                    .minor = static_cast<u32>(_ctx->ver.min)};
}

std::string_view gl_context::renderer_string() const {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  return _ctx->renderer_string;
}

std::string_view gl_context::vendor_string() const {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  return _ctx->vendor_string;
}

std::string_view gl_context::version_string() const {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
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

void setup_vertex_attributes(gl_context& gl, const gl_vertex_layout& layout,
                             span<const gl_draw_command::vertex_binding> vertex_buffers) {
  const auto attribs = layout.attributes();
  SHOGLE_ASSERT(!attribs.empty());

  const auto bind_attrib_pointer = [&](shogle::attribute_type type, u32 location, size_t offset_) {
    void* offset = reinterpret_cast<void*>(offset_);
    const u32 dimension = attribute_dimension(type);
    const auto underlying = underlying_attribute_type(type);

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
        SHOGLE_UNREACHABLE();
    }
  };

  GL_ASSERT(glBindVertexArray(layout.vao()));
  if (layout.type() == gl_vertex_layout::TYPE_AOS_LAYOUT) {
    SHOGLE_ASSERT(vertex_buffers.size() == 1,
                  "AOS vertex layouts uses only a single vertex buffer");
    GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[0].buffer));
    for (const auto& attrib : attribs) {
      SHOGLE_ASSERT(attrib.location < gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS,
                    "Attribute location out of range");
      bind_attrib_pointer(attrib.type, attrib.location, attrib.offset);
    }
  } else {
    SHOGLE_ASSERT(vertex_buffers.size() == attribs.size(),
                  "SOA vertex layout needs equal number of vertex buffers and attributes");
    SHOGLE_ASSERT(vertex_buffers.size() >= gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS,
                  "Vertex buffer count out ofr attribute range");

    std::array<GLuint, gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS> bind_map{};
    for (const auto [buffer, location] : vertex_buffers) {
      SHOGLE_ASSERT(location < gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS,
                    "Vertex buffer binding out of range");
      bind_map[location] = buffer;
    }
    for (const auto& attrib : attribs) {
      SHOGLE_ASSERT(attrib.location < gl_vertex_layout::MAX_ATTRIBUTE_BINDINGS,
                    "Attribute location out of range");
      const GLuint buffer = bind_map[attrib.location];
      if (buffer == 0) {
        continue;
      }
      GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, buffer));
      bind_attrib_pointer(attrib.type, attrib.location, attrib.offset);
    }
  }
}

void upload_uniforms(gl_context& gl, span<const gl_draw_command::push_uniform> uniforms) {
  for (const auto& [data, type, location] : uniforms) {
    switch (type) {
      case attribute_type::f32: {
        f32 val;
        std::memcpy(&val, &data[0], sizeof(f32));
        GL_ASSERT(glUniform1f(location, val));
      } break;
      case attribute_type::vec2: {
        const f32* ptr = reinterpret_cast<const f32*>(&data[0]);
        GL_ASSERT(glUniform2fv(location, 1, ptr));
      } break;
      case attribute_type::vec3: {
        const f32* ptr = reinterpret_cast<const f32*>(&data[0]);
        GL_ASSERT(glUniform3fv(location, 1, ptr));
      } break;
      case attribute_type::vec4: {
        const f32* ptr = reinterpret_cast<const f32*>(&data[0]);
        GL_ASSERT(glUniform4fv(location, 1, ptr));
      } break;
      case attribute_type::mat3: {
        const f32* ptr = reinterpret_cast<const f32*>(&data[0]);
        GL_ASSERT(glUniformMatrix3fv(location, 1, GL_FALSE, ptr));
      } break;
      case attribute_type::mat4: {
        const f32* ptr = reinterpret_cast<const f32*>(&data[0]);
        GL_ASSERT(glUniformMatrix4fv(location, 1, GL_FALSE, ptr));
      } break;
      case attribute_type::f64: {
        f64 val;
        std::memcpy(&val, &data[0], sizeof(f64));
        GL_ASSERT(glUniform1d(location, val));
      } break;
      case attribute_type::dvec2: {
        const f64* ptr = reinterpret_cast<const f64*>(&data[0]);
        GL_ASSERT(glUniform2dv(location, 1, ptr));
      } break;
      case attribute_type::dvec3: {
        const f64* ptr = reinterpret_cast<const f64*>(&data[0]);
        GL_ASSERT(glUniform3dv(location, 1, ptr));
      } break;
      case attribute_type::dvec4: {
        const f64* ptr = reinterpret_cast<const f64*>(&data[0]);
        GL_ASSERT(glUniform4dv(location, 1, ptr));
      } break;
      case attribute_type::i32: {
        i32 val;
        std::memcpy(&val, &data[0], sizeof(i32));
        GL_ASSERT(glUniform1i(location, val));
      } break;
      case attribute_type::ivec2: {
        const i32* ptr = reinterpret_cast<const i32*>(&data[0]);
        GL_ASSERT(glUniform2iv(location, 1, ptr));
      } break;
      case attribute_type::ivec3: {
        const i32* ptr = reinterpret_cast<const i32*>(&data[0]);
        GL_ASSERT(glUniform3iv(location, 1, ptr));
      } break;
      case attribute_type::ivec4: {
        const i32* ptr = reinterpret_cast<const i32*>(&data[0]);
        GL_ASSERT(glUniform4iv(location, 1, ptr));
      } break;
      case attribute_type::u32: {
        u32 val;
        std::memcpy(&val, &data[0], sizeof(u32));
        GL_ASSERT(glUniform1ui(location, val));
      } break;
      case attribute_type::uvec2: {
        const u32* ptr = reinterpret_cast<const u32*>(&data[0]);
        GL_ASSERT(glUniform2uiv(location, 1, ptr));
      } break;
      case attribute_type::uvec3: {
        const u32* ptr = reinterpret_cast<const u32*>(&data[0]);
        GL_ASSERT(glUniform3uiv(location, 1, ptr));
      } break;
      case attribute_type::uvec4: {
        const u32* ptr = reinterpret_cast<const u32*>(&data[0]);
        GL_ASSERT(glUniform4uiv(location, 1, ptr));
      } break;
    };
  }
}

} // namespace

void gl_context::start_frame(const gl_clear_opts& clear) {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  auto& gl = *this;

  const auto clear_framebuffer = [&](GLuint fbo, const color4& color, GLbitfield clear_flags,
                                     const rectangle_pos<u32>& viewport) {
    GL_ASSERT(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo));
    GL_ASSERT(glViewport(viewport.x, viewport.y, viewport.width, viewport.height));
    GL_ASSERT(glClearColor(color.r, color.g, color.b, color.a));
    GL_ASSERT(glClear(clear_flags));
  };
  const auto viewport = [&]() -> rectangle_pos<u32> {
    if (clear.viewport) {
      return *clear.viewport;
    } else {
      const auto [w, h] = provider().surface_extent();
      return {0, 0, w, h};
    }
  }();

  clear_framebuffer(DEFAULT_FRAMEBUFFER, clear.clear_color, clear.clear_flags, viewport);
  for (const auto& [clear_color, viewport, clear_flags, fbo] : clear.fbos) {
    clear_framebuffer(fbo, clear_color, clear_flags, viewport);
  }
}

void gl_context::submit_command(const gl_draw_command& cmd,
                                ptr_view<const gl_framebuffer> target) {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  auto& gl = *this;
  const gl_graphics_pipeline& pipeline = *cmd.pipeline;
  const auto primitive = pipeline.primitive();

  const auto bind_shader_buffers = [&]() {
    for (const auto [buffer, type, offset, size, location] : cmd.shader_bindings) {
      GL_ASSERT(glBindBufferRange(type, location, buffer, offset, size));
    }
  };
  const auto bind_textures = [&]() {
    for (const auto& [texture, type, index] : cmd.texture_bindings) {
      GL_ASSERT(glActiveTexture(GL_TEXTURE0 + index));
      GL_ASSERT(glBindTexture(type, texture));
    }
  };

  const auto draw_arrays = [&]() {
    GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_DEFAULT_BINDING));
    if (cmd.instances > 1) {
      GL_ASSERT(
        glDrawArraysInstanced(primitive, cmd.vertex_offset, cmd.draw_count, cmd.instances));
    } else {
      GL_ASSERT(glDrawArrays(primitive, cmd.vertex_offset, cmd.draw_count));
    }
  };

  const auto draw_indexed = [&]() {
    SHOGLE_ASSERT(cmd.index_bind.has_value());
    static constexpr auto idx_formats = std::to_array<gldefs::GLenum>({
      0x1400, // GL_BYTE
      0x1401, // GL_UNSIGNED_BYTE
      0x1402, // GL_SHORT
      0x1403, // GL_UNSIGNED_SHORT
      0x1404, // GL_INT
      0x1405, // GL_UNSIGNED_INT
    });
    static constexpr auto idx_sizes = std::to_array<size_t>({
      sizeof(i8),  // GL_BYTE
      sizeof(u8),  // GL_UNSIGNED_BYTE
      sizeof(i16), // GL_SHORT
      sizeof(u16), // GL_UNSIGNED_SHORT
      sizeof(i32), // GL_INT
      sizeof(u32), // GL_UNSIGNED_INT
    });
    SHOGLE_ASSERT(cmd.index_bind->format < idx_formats.size(), "Invalid index buffer format");
    const u32 format_idx = cmd.index_bind->format;
    GL_ASSERT(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cmd.index_bind->buffer));

    const void* idx_offset =
      reinterpret_cast<const void*>(cmd.index_bind->index_offset * idx_sizes[format_idx]);
    const gldefs::GLenum format = idx_formats[format_idx];
    if (cmd.instances > 1) {
      GL_ASSERT(glDrawElementsInstancedBaseVertex(primitive, cmd.draw_count, format, idx_offset,
                                                  cmd.instances, cmd.vertex_offset));

    } else {
      GL_ASSERT(glDrawElementsBaseVertex(primitive, cmd.draw_count, format, idx_offset,
                                         cmd.vertex_offset));
    }
  };

  const auto viewport = [&]() -> rectangle_pos<u32> {
    if (cmd.viewport.has_value()) {
      return *cmd.viewport;
    } else {
      const auto [w, h] = provider().surface_extent();
      return {0, 0, w, h};
    }
  }();
  const auto scissor = cmd.scissor ? *cmd.scissor : viewport;
  setup_framebuffer(gl, target.empty() ? DEFAULT_FRAMEBUFFER : target->id(), viewport, scissor);
  GL_ASSERT(glUseProgram(pipeline.program()));
  setup_render_state(gl, pipeline.depth_test(), pipeline.stencil_test(), pipeline.blending(),
                     pipeline.culling(), pipeline.poly_mode(), pipeline.poly_width());

  setup_vertex_attributes(gl, cmd.vertex_layout, cmd.vertex_bindings);
  bind_shader_buffers();
  bind_textures();
  upload_uniforms(gl, cmd.uniforms);
  if (cmd.index_bind.has_value()) {
    draw_indexed();
  } else {
    draw_arrays();
  }
}

void gl_context::submit_command(const gl_external_command& cmd,
                                ptr_view<const gl_framebuffer> target) {
  SHOGLE_ASSERT(_ctx, "gl_context use after free");
  const GLuint fbo = target.empty() ? DEFAULT_FRAMEBUFFER : target->id();
  setup_framebuffer(*this, fbo, cmd.viewport, cmd.scissor);
  setup_render_state(*this, cmd.depth_test, cmd.stencil_test, cmd.blending, cmd.culling,
                     cmd.poly_mode, cmd.poly_width);
  std::invoke(cmd.callback, *this, fbo);
}

void gl_context::end_frame() {
  // No-op
}

void gl_context::destroy() noexcept {
  if (SHOGLE_UNLIKELY(!_ctx)) {
    return;
  }
  _ctx.reset();
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
