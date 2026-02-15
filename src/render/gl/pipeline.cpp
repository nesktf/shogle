#include "./context_private.hpp"
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/pipeline.hpp>

namespace shogle {

gl_shader::gl_shader(create_t, gldefs::GLhandle id, shader_stage stage) : _id(id), _stage(stage) {}

gl_shader::gl_shader(gl_context& gl, std::string_view src, shader_stage stage) :
    gl_shader(::shogle::gl_shader::create(gl, src, stage).value()) {}

namespace {

std::string_view shader_name(gl_shader::shader_stage stage) {
#define STR(enum_)               \
  case gl_shader::STAGE_##enum_: \
    return #enum_
  switch (stage) {
    STR(VERTEX);
    STR(FRAGMENT);
    STR(GEOMETRY);
    STR(TESS_CTRL);
    STR(TESS_EVAL);
    STR(COMPUTE);
    default:
      return "UNKNOWN";
  }
#undef STR
}

} // namespace

gl_s_expect<gl_shader> gl_shader::create(gl_context& gl, std::string_view src,
                                         shader_stage stage) {
  gldefs::GLhandle shader = GL_ASSERT_RET(glCreateShader(stage));

  const char* src_data = src.data();
  const GLint len = static_cast<GLint>(src.size());
  GL_ASSERT(glShaderSource(shader, 1, &src_data, &len));
  GL_CALL(glCompileShader(shader)); // No assert

  int succ;
  GL_ASSERT(glGetShaderiv(shader, GL_COMPILE_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_ASSERT(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &err_len));
    char log_buffer[1024] = {0};
    GL_ASSERT(glGetShaderInfoLog(shader, 1024, &err_len, &log_buffer[0]));
    GL_ASSERT(glDeleteShader(shader));
    std::string_view buffer_view(log_buffer, std::min(err_len, 1024));
    SHOGLE_GL_LOG(error, "Shader compilation failed: {}", buffer_view);
    return {unexpect, fmt::format("Shader compilation failed: {}", buffer_view)};
  }
  SHOGLE_GL_LOG(verbose, "Shader created ({}) (type: {})", shader, shader_name(stage));
  return {in_place, create_t{}, shader, stage};
}

void gl_shader::destroy_n(gl_context& gl, gl_shader* shaders, size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!shaders)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(shaders[i].invalidated())) {
      continue;
    }
    SHOGLE_GL_LOG(verbose, "Shader destroyed ({}) (type: {})", shaders[i]._id,
                  shader_name(shaders[i]._stage));
    GL_CALL(glDeleteShader(shaders[i]._id));
    shaders[i]._id = GL_NULL_HANDLE;
  }
}

void gl_shader::destroy_n(gl_context& gl, span<gl_shader> shaders) noexcept {
  destroy_n(gl, shaders.data(), shaders.size());
}

void gl_shader::destroy(gl_context& gl, gl_shader& shader) noexcept {
  if (SHOGLE_UNLIKELY(shader.invalidated())) {
    return;
  }
  SHOGLE_GL_LOG(verbose, "Shader destroyed ({}) (type: {})", shader._id,
                shader_name(shader._stage));
  GL_CALL(glDeleteShader(shader._id));
  shader._id = GL_NULL_HANDLE;
}

gldefs::GLhandle gl_shader::id() const {
  SHOGLE_ASSERT(!invalidated(), "gl_shader use after free");
  return _id;
}

gl_shader::shader_stage gl_shader::stage() const {
  SHOGLE_ASSERT(!invalidated(), "gl_shader use after free");
  return _stage;
}

bool gl_shader::invalidated() const noexcept {
  return _id == GL_NULL_HANDLE;
}

namespace {

constexpr u32 map_shader(gl_shader::shader_stage stage) noexcept {
  return static_cast<u32>(stage) % gl_shader_builder::MAP_SIZE;
}

// Make sure everything has an unique index
static_assert(map_shader(gl_shader::STAGE_FRAGMENT) == 4);
static_assert(map_shader(gl_shader::STAGE_GEOMETRY) == 1);
static_assert(map_shader(gl_shader::STAGE_VERTEX) == 5);
static_assert(map_shader(gl_shader::STAGE_TESS_EVAL) == 7);
static_assert(map_shader(gl_shader::STAGE_TESS_CTRL) == 8);

} // namespace

gl_shader_builder::gl_shader_builder() noexcept {
  std::memset(_shaders.data(), 0, MAP_SIZE * sizeof(_shaders[0]));
}

gl_shader_builder& gl_shader_builder::add_shader(const gl_shader& shader) {
  SHOGLE_ASSERT(shader.stage() != gl_shader::STAGE_COMPUTE,
                "Can't use compute shaders in a graphics pipeline");
  _shaders[map_shader(shader.stage())] = shader.id();
  return *this;
}

gldefs::GLhandle gl_shader_builder::get_shader(gl_shader::shader_stage stage) {
  return _shaders[map_shader(stage)];
}

gl_shader::graphics_set gl_shader_builder::build() const {
  const auto vert = _shaders[map_shader(gl_shader::STAGE_VERTEX)];
  SHOGLE_ASSERT(vert, "No vertex shader");

  const auto frag = _shaders[map_shader(gl_shader::STAGE_FRAGMENT)];
  SHOGLE_ASSERT(frag, "No fragment shaer");

  std::array<gldefs::GLhandle, 5u> shader_set;
  u32 shader_count = 0;
  gldefs::GLbitfield stages = gl_shader::STAGE_NO_BITS;
  const auto add_shader = [&](gldefs::GLhandle shader, gl_shader::stages_bits bits) {
    SHOGLE_ASSERT(shader_count < shader_set.size());
    shader_set[shader_count++] = shader;
    stages |= bits;
  };
  add_shader(vert, gl_shader::STAGE_VERTEX_BIT);
  add_shader(frag, gl_shader::STAGE_FRAGMENT_BIT);
  if (auto geom = _shaders[map_shader(gl_shader::STAGE_GEOMETRY)]; geom) {
    add_shader(geom, gl_shader::STAGE_GEOMETRY_BIT);
  }

  // For tesselation, the evaluation stage is always required
  // and the control stage is optional
  const auto eval = _shaders[map_shader(gl_shader::STAGE_TESS_EVAL)];
  const auto ctrl = _shaders[map_shader(gl_shader::STAGE_TESS_CTRL)];
  if (ctrl) {
    add_shader(ctrl, gl_shader::STAGE_TESS_CTRL_BIT);
    if (eval) {
      add_shader(eval, gl_shader::STAGE_TESS_EVAL_BIT);
    }
  }

  SHOGLE_ASSERT(shader_count >= 2 && shader_count <= 5);
  return {shader_set, shader_count, stages};
}

gl_graphics_pipeline::gl_graphics_pipeline(create_t, gldefs::GLhandle program,
                                           gldefs::GLbitfield stages) :
    _stencil(::shogle::gl_stencil_test_props::make_default(false)),
    _depth(::shogle::gl_depth_test_props::make_default(false)),
    _blending(::shogle::gl_blending_props::make_default(false)),
    _culling(::shogle::gl_culling_props::make_default(false)), _stages(stages), _program(program),
    _primitive(PRIMITIVE_TRIANGLES), _poly_mode(POLY_MODE_FILL), _poly_width(1.f) {}

gl_graphics_pipeline::gl_graphics_pipeline(gl_context& gl,
                                           const gl_shader::graphics_set& shaders) :
    gl_graphics_pipeline(::shogle::gl_graphics_pipeline::create(gl, shaders).value()) {}

gl_s_expect<gl_graphics_pipeline>
gl_graphics_pipeline::create(gl_context& gl, const gl_shader::graphics_set& shaders) {
  const auto shader_span = shaders.stages();
  SHOGLE_ASSERT(!shader_span.empty(), "No shaders in set");
  SHOGLE_ASSERT(shader_span.size() >= 2 && shader_span.size() <= 5, "Invalid shader set");
  SHOGLE_ASSERT(shaders.active_stages() & gl_shader::STAGE_VERTEX_BIT, "No vertex shader in set");
  SHOGLE_ASSERT(shaders.active_stages() & gl_shader::STAGE_FRAGMENT_BIT,
                "No fragment shader in set");

  gldefs::GLhandle program = GL_ASSERT_RET(glCreateProgram());
  for (const gldefs::GLhandle shader : shader_span) {
    GL_ASSERT(glAttachShader(program, shader));
  }
  GL_CALL(glLinkProgram(program));

  int succ;
  GL_ASSERT(glGetProgramiv(program, GL_LINK_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_ASSERT(glGetShaderiv(shader_span[0], GL_INFO_LOG_LENGTH, &err_len));
    char log_buffer[1024] = {0};
    GL_ASSERT(glGetShaderInfoLog(shader_span[0], 1024, &err_len, &log_buffer[0]));
    GL_ASSERT(glDeleteProgram(program));
    std::string_view buffer_view(log_buffer, std::min(err_len, 1024));
    SHOGLE_GL_LOG(error, "Program linking failed: {}", buffer_view);
    return {unexpect, fmt::format("Program linking failed: {}", buffer_view)};
  }

  for (const gldefs::GLhandle shader : shader_span) {
    GL_ASSERT(glDettachShader(program, shader));
  }
  SHOGLE_GL_LOG(verbose, "Pipeline created ({})", program);
  return {in_place, create_t{}, program, shaders.active_stages()};
}

void gl_graphics_pipeline::destroy(gl_context& gl, gl_graphics_pipeline& pipeline) noexcept {
  if (SHOGLE_UNLIKELY(pipeline.invalidated())) {
    return;
  }
  GL_CALL(glDeleteProgram(pipeline._program));
  SHOGLE_GL_LOG(verbose, "Pipeline destroyed ({})", pipeline._program);
  pipeline._program = GL_NULL_HANDLE;
}

void gl_graphics_pipeline::destroy_n(gl_context& gl, gl_graphics_pipeline* pipelines,
                                     size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!pipelines)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(pipelines[i].invalidated())) {
      continue;
    }
    SHOGLE_GL_LOG(verbose, "Pipeline destroyed ({})", pipelines[i]._program);
    GL_CALL(glDeleteProgram(pipelines[i]._program));
    pipelines[i]._program = GL_NULL_HANDLE;
  }
}

void gl_graphics_pipeline::destroy_n(gl_context& gl,
                                     span<gl_graphics_pipeline> pipelines) noexcept {
  destroy_n(gl, pipelines.data(), pipelines.size());
}

optional<u32> gl_graphics_pipeline::uniform_location(gl_context& gl, const char* name) const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  const gldefs::GLint loc = GL_ASSERT_RET(glGetUniformLocation(_program, name));
  if (loc == -1) {
    return {nullopt};
  } else {
    return {in_place, static_cast<u32>(loc)};
  }
}

u32 gl_graphics_pipeline::uniform_count(gl_context& gl) const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  gldefs::GLint count;
  GL_ASSERT(glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &count));
  return static_cast<u32>(count);
}

auto gl_graphics_pipeline::query_uniform_index(gl_context& gl, shader_attrib_props& props,
                                               u32 idx) const -> shader_attrib_type {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  gldefs::GLint size;
  gldefs::GLsizei len;
  gldefs::GLenum type;
  const auto err = GL_RET_ERR(glGetActiveUniform(_program, static_cast<GLuint>(idx), MAX_NAME_SIZE,
                                                 &len, &size, &type, props.name));
  if (err) {
    return TYPE_NONE;
  }
  props.name_len = std::min(static_cast<size_t>(len), MAX_NAME_SIZE);
  props.type = static_cast<shader_attrib_type>(type);
  props.size = static_cast<size_t>(size);
  return props.type;
}

optional<u32> gl_graphics_pipeline::attribute_location(gl_context& gl, const char* name) const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  const gldefs::GLint loc = GL_ASSERT_RET(glGetAttribLocation(_program, name));
  if (loc == -1) {
    return {nullopt};
  } else {
    return {in_place, static_cast<u32>(loc)};
  }
}

u32 gl_graphics_pipeline::attribute_count(gl_context& gl) const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  gldefs::GLint count;
  GL_ASSERT(glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &count));
  return static_cast<u32>(count);
}

auto gl_graphics_pipeline::query_attribute_index(gl_context& gl, shader_attrib_props& props,
                                                 u32 idx) const -> shader_attrib_type {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  gldefs::GLint size;
  gldefs::GLsizei len;
  gldefs::GLenum type;
  const auto err = GL_RET_ERR(glGetActiveAttrib(_program, static_cast<GLuint>(idx), MAX_NAME_SIZE,
                                                &len, &size, &type, props.name));
  if (err) {
    return TYPE_NONE;
  }
  props.name_len = std::min(static_cast<size_t>(len), MAX_NAME_SIZE);
  props.type = static_cast<shader_attrib_type>(type);
  props.size = static_cast<size_t>(size);
  return props.type;
}

gl_graphics_pipeline& gl_graphics_pipeline::reset_props() {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _primitive = PRIMITIVE_TRIANGLES;
  _poly_mode = POLY_MODE_FILL;
  _poly_width = 1.f;
  _stencil = ::shogle::gl_stencil_test_props::make_default(false);
  _depth = ::shogle::gl_depth_test_props::make_default(false);
  _blending = ::shogle::gl_blending_props::make_default(false);
  _culling = ::shogle::gl_culling_props::make_default(false);
  return *this;
}

gl_graphics_pipeline& gl_graphics_pipeline::set_primitive(primitive_mode primitive) {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _primitive = primitive;
  return *this;
}

gl_graphics_pipeline& gl_graphics_pipeline::set_poly_mode(polygon_mode poly_mode) {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _poly_mode = poly_mode;
  return *this;
}

gl_graphics_pipeline& gl_graphics_pipeline::set_poly_width(f32 poly_width) {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _poly_width = poly_width;
  return *this;
}

gl_graphics_pipeline&
gl_graphics_pipeline::set_stencil_test(const gl_stencil_test_props& stencil) {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _stencil = stencil;
  return *this;
}

gl_graphics_pipeline& gl_graphics_pipeline::set_depth_test(const gl_depth_test_props& depth) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_graphics_pipeline use after free");
  _depth = depth;
  return *this;
}

gl_graphics_pipeline& gl_graphics_pipeline::set_blending(const gl_blending_props& blending) {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _blending = blending;
  return *this;
}

gl_graphics_pipeline& gl_graphics_pipeline::set_culling(const gl_culling_props& culling) {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  _culling = culling;
  return *this;
}

gldefs::GLhandle gl_graphics_pipeline::program() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _program;
}

gldefs::GLbitfield gl_graphics_pipeline::stages() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _stages;
}

auto gl_graphics_pipeline::primitive() const -> primitive_mode {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _primitive;
}

auto gl_graphics_pipeline::poly_mode() const -> polygon_mode {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _poly_mode;
}

f32 gl_graphics_pipeline::poly_width() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _poly_width;
}

const gl_stencil_test_props& gl_graphics_pipeline::stencil_test() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _stencil;
}

const gl_depth_test_props& gl_graphics_pipeline::depth_test() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _depth;
}

const gl_blending_props& gl_graphics_pipeline::blending() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _blending;
}

const gl_culling_props& gl_graphics_pipeline::culling() const {
  SHOGLE_ASSERT(!invalidated(), "gl_graphics_pipeline use after free");
  return _culling;
}

bool gl_graphics_pipeline::invalidated() const noexcept {
  return _program == GL_NULL_HANDLE;
}

} // namespace shogle
