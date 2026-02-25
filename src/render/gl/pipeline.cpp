#include "./context_private.hpp"
#include <shogle/render/gl/context.hpp>
#include <shogle/render/gl/pipeline.hpp>

namespace shogle {

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

constexpr auto shader_map = std::to_array<gldefs::GLenum>({
  0x8B31, // GL_VERTEX_SHADER
  0x8B30, // GL_FRAGMENT_SHADER
  0x8DD9, // GL_GEOMETRY_SHADER
  0x8E87, // GL_TESS_EVALUATION_SHADER
  0x8E88, // GL_TESS_CONTROL_SHADER
  0x91B9, // GL_COMPUTE_SHADER
});

constexpr auto shader_bit_map = std::to_array<gldefs::GLbitfield>({
  gl_shader::STAGE_VERTEX_BIT,
  gl_shader::STAGE_FRAGMENT_BIT,
  gl_shader::STAGE_GEOMETRY_BIT,
  gl_shader::STAGE_TESS_CTRL_BIT,
  gl_shader::STAGE_TESS_EVAL_BIT,
});

} // namespace

gl_pipeline_builder::gl_pipeline_builder() noexcept :
    _set{{0, 0, 0, 0, 0}, 0, 0}, _stencil(::shogle::gl_stencil_test_props::make_default(false)),
    _depth(::shogle::gl_depth_test_props::make_default(false)),
    _blending(::shogle::gl_blending_props::make_default(false)),
    _culling(::shogle::gl_culling_props::make_default(false)),
    _primitive(shogle::gl_pipeline::PRIMITIVE_TRIANGLES),
    _poly_mode(shogle::gl_pipeline::POLY_MODE_FILL), _poly_width(1.f) {}

gl_pipeline_builder& gl_pipeline_builder::add_shader(const gl_shader& shader) {
  SHOGLE_ASSERT(shader.stage() != gl_shader::STAGE_COMPUTE,
                "Can't use compute shaders in a graphics pipeline");
  SHOGLE_ASSERT((size_t)shader.stage() < shader_map.size());
  if (_set.shaders[(size_t)shader.stage()] == 0) {
    ++_set.shader_count;
  }
  _set.shaders[(size_t)shader.stage()] = shader.id();
  _set.active_stages |= shader_bit_map[(size_t)shader.stage()];
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_depth_test(const gl_depth_test_props& depth) {
  _depth = depth;
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_stencil_test(const gl_stencil_test_props& stencil) {
  _stencil = stencil;
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_blending(const gl_blending_props& blending) {
  _blending = blending;
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_culling(const gl_culling_props& culling) {
  _culling = culling;
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_primitive(gl_pipeline::primitive_mode primitive) {
  _primitive = primitive;
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_polygon_mode(gl_pipeline::polygon_mode poly_mode) {
  _poly_mode = poly_mode;
  return *this;
}

gl_pipeline_builder& gl_pipeline_builder::set_polygon_width(f32 poly_width) {
  _poly_width = poly_width;
  return *this;
}

void gl_pipeline_builder::reset() {
  _set.shaders[0] = 0;
  _set.shaders[1] = 0;
  _set.shaders[2] = 0;
  _set.shaders[3] = 0;
  _set.shaders[4] = 0;
  _set.shader_count = 0;
  _set.active_stages = 0;
  _stencil = ::shogle::gl_stencil_test_props::make_default(false);
  _depth = ::shogle::gl_depth_test_props::make_default(false);
  _blending = ::shogle::gl_blending_props::make_default(false);
  _culling = ::shogle::gl_culling_props::make_default(false);
  _primitive = shogle::gl_pipeline::PRIMITIVE_TRIANGLES;
  _poly_mode = shogle::gl_pipeline::POLY_MODE_FILL;
  _poly_width = 1.f;
}

gl_s_expect<gl_pipeline> gl_pipeline_builder::build(gl_context& gl) const {
  const gl_pipeline::pipeline_props props{
    .stencil = _stencil,
    .depth = _depth,
    .blending = _blending,
    .culling = _culling,
  };
  return gl_pipeline::create(gl, _set, _primitive, _poly_mode, _poly_width, props);
}

gl_shader::gl_shader(create_t, gldefs::GLhandle id, shader_stage stage) : _id(id), _stage(stage) {}

gl_shader::gl_shader(gl_context& gl, const char* src, size_t src_size, shader_stage stage) :
    gl_shader(::shogle::gl_shader::create(gl, src, src_size, stage).value()) {}

gl_s_expect<gl_shader> gl_shader::create(gl_context& gl, const char* src, size_t src_size,
                                         shader_stage stage) {
  SHOGLE_ASSERT((size_t)stage < shader_map.size());
  gldefs::GLhandle shader = GL_ASSERT_RET(glCreateShader(shader_map[(size_t)stage]));

  const GLint len = static_cast<GLint>(src_size);
  GL_ASSERT(glShaderSource(shader, 1, &src, &len));
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
    SHOGLE_GL_LOG(ERROR, "SHADER_COMPILER ({}) {}", shader, buffer_view);
    return {unexpect, fmt::format("Shader compilation failed: {}", buffer_view)};
  }
  SHOGLE_GL_LOG(VERBOSE, "SHADER_CREATE ({}) [type: {}]", shader, shader_name(stage));
  return {in_place, create_t{}, shader, stage};
}

void gl_shader::destroy_n(gl_context& gl, gl_shader* shaders, size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!shaders)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(shaders[i]._id == GL_NULL_HANDLE)) {
      continue;
    }
    SHOGLE_GL_LOG(VERBOSE, "SHADER_DESTROY ({}) [type: {}]", shaders[i]._id,
                  shader_name(shaders[i]._stage));
    GL_CALL(glDeleteShader(shaders[i]._id));
    shaders[i]._id = GL_NULL_HANDLE;
  }
}

void gl_shader::destroy_n(gl_context& gl, span<gl_shader> shaders) noexcept {
  destroy_n(gl, shaders.data(), shaders.size());
}

void gl_shader::destroy(gl_context& gl, gl_shader& shader) noexcept {
  if (SHOGLE_UNLIKELY(shader._id == GL_NULL_HANDLE)) {
    return;
  }
  SHOGLE_GL_LOG(VERBOSE, "SHADER_DESTROY ({}) [type: {}]", shader._id, shader_name(shader._stage));
  GL_CALL(glDeleteShader(shader._id));
  shader._id = GL_NULL_HANDLE;
}

gldefs::GLhandle gl_shader::id() const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_shader use after free");
  return _id;
}

gl_shader::shader_stage gl_shader::stage() const {
  SHOGLE_ASSERT(_id != GL_NULL_HANDLE, "gl_shader use after free");
  return _stage;
}

gl_pipeline::gl_pipeline(create_t, gldefs::GLhandle program, gldefs::GLbitfield stages,
                         primitive_mode primitive, polygon_mode poly, f32 poly_width,
                         ptr_view<const pipeline_props> props) :
    _stencil(props ? props->stencil : ::shogle::gl_stencil_test_props::make_default(false)),
    _depth(props ? props->depth : ::shogle::gl_depth_test_props::make_default(false)),
    _blending(props ? props->blending : ::shogle::gl_blending_props::make_default(false)),
    _culling(props ? props->culling : ::shogle::gl_culling_props::make_default(false)),
    _stages(stages), _program(program), _primitive(primitive), _poly_mode(poly),
    _poly_width(poly_width) {}

gl_pipeline::gl_pipeline(gl_context& gl, const shader_set& shaders, primitive_mode primitive,
                         polygon_mode poly, f32 poly_width, ptr_view<const pipeline_props> props) :
    gl_pipeline(
      ::shogle::gl_pipeline::create(gl, shaders, primitive, poly, poly_width, props).value()) {}

gl_s_expect<gl_pipeline> gl_pipeline::create(gl_context& gl, const shader_set& shaders,
                                             primitive_mode primitive, polygon_mode poly,
                                             f32 poly_width,
                                             ptr_view<const pipeline_props> props) {
  SHOGLE_ASSERT(shaders.shader_count >= 2 && shaders.shader_count <= 5);
  SHOGLE_ASSERT(shaders.active_stages & gl_shader::STAGE_VERTEX_BIT, "No vertex shader");
  SHOGLE_ASSERT(shaders.active_stages & gl_shader::STAGE_FRAGMENT_BIT, "No fragment shader");
  if (shaders.active_stages & gl_shader::STAGE_TESS_CTRL_BIT) {
    SHOGLE_ASSERT(shaders.active_stages & gl_shader::STAGE_TESS_EVAL, "No tess. eval shader");
  }

  gldefs::GLhandle program = GL_ASSERT_RET(glCreateProgram());
  for (size_t i = 0; i < (size_t)shaders.shader_count; ++i) {
    SHOGLE_ASSERT(shaders.shaders[i] != 0);
    GL_ASSERT(glAttachShader(program, shaders.shaders[i]));
  }
  GL_CALL(glLinkProgram(program));

  int succ;
  GL_ASSERT(glGetProgramiv(program, GL_LINK_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_ASSERT(glGetShaderiv(shaders.shaders[0], GL_INFO_LOG_LENGTH, &err_len));
    char log_buffer[1024] = {0};
    GL_ASSERT(glGetShaderInfoLog(shaders.shaders[0], 1024, &err_len, &log_buffer[0]));
    GL_ASSERT(glDeleteProgram(program));
    std::string_view buffer_view(log_buffer, std::min(err_len, 1024));
    SHOGLE_GL_LOG(ERROR, "PIPELINE_LINKER ({}) {}", program, buffer_view);
    return {unexpect, fmt::format("Program linking failed: {}", buffer_view)};
  }

  for (size_t i = 0; i < (size_t)shaders.shader_count; ++i) {
    GL_ASSERT(glDettachShader(program, shaders.shaders[i]));
  }
  SHOGLE_GL_LOG(VERBOSE, "PIPELINE_CREATE ({})", program);
  return {in_place,  create_t{}, program,    shaders.active_stages,
          primitive, poly,       poly_width, props};
}

void gl_pipeline::destroy(gl_context& gl, gl_pipeline& pipeline) noexcept {
  if (SHOGLE_UNLIKELY(pipeline._program == GL_NULL_HANDLE)) {
    return;
  }
  GL_CALL(glDeleteProgram(pipeline._program));
  SHOGLE_GL_LOG(VERBOSE, "PIPELINE_DESTROY ({})", pipeline._program);
  pipeline._program = GL_NULL_HANDLE;
}

void gl_pipeline::destroy_n(gl_context& gl, gl_pipeline* pipelines, size_t count) noexcept {
  if (SHOGLE_UNLIKELY(!pipelines)) {
    return;
  }
  for (size_t i = 0; i < count; ++i) {
    if (SHOGLE_UNLIKELY(pipelines[i]._program == GL_NULL_HANDLE)) {
      continue;
    }
    SHOGLE_GL_LOG(VERBOSE, "PIPELINE_DESTROY ({})", pipelines[i]._program);
    GL_CALL(glDeleteProgram(pipelines[i]._program));
    pipelines[i]._program = GL_NULL_HANDLE;
  }
}

void gl_pipeline::destroy_n(gl_context& gl, span<gl_pipeline> pipelines) noexcept {
  destroy_n(gl, pipelines.data(), pipelines.size());
}

optional<u32> gl_pipeline::uniform_location(gl_context& gl, const char* name) const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  const gldefs::GLint loc = GL_ASSERT_RET(glGetUniformLocation(_program, name));
  if (loc == -1) {
    return {nullopt};
  } else {
    return {in_place, static_cast<u32>(loc)};
  }
}

u32 gl_pipeline::uniform_count(gl_context& gl) const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  gldefs::GLint count;
  GL_ASSERT(glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &count));
  return static_cast<u32>(count);
}

auto gl_pipeline::query_uniform_index(gl_context& gl, shader_attrib_props& props, u32 idx) const
  -> shader_attrib_type {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
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

optional<u32> gl_pipeline::attribute_location(gl_context& gl, const char* name) const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  const gldefs::GLint loc = GL_ASSERT_RET(glGetAttribLocation(_program, name));
  if (loc == -1) {
    return {nullopt};
  } else {
    return {in_place, static_cast<u32>(loc)};
  }
}

u32 gl_pipeline::attribute_count(gl_context& gl) const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  gldefs::GLint count;
  GL_ASSERT(glGetProgramiv(_program, GL_ACTIVE_ATTRIBUTES, &count));
  return static_cast<u32>(count);
}

auto gl_pipeline::query_attribute_index(gl_context& gl, shader_attrib_props& props, u32 idx) const
  -> shader_attrib_type {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
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

gl_pipeline& gl_pipeline::reset_props() {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _primitive = PRIMITIVE_TRIANGLES;
  _poly_mode = POLY_MODE_FILL;
  _poly_width = 1.f;
  _stencil = ::shogle::gl_stencil_test_props::make_default(false);
  _depth = ::shogle::gl_depth_test_props::make_default(false);
  _blending = ::shogle::gl_blending_props::make_default(false);
  _culling = ::shogle::gl_culling_props::make_default(false);
  return *this;
}

gl_pipeline& gl_pipeline::set_primitive(primitive_mode primitive) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _primitive = primitive;
  return *this;
}

gl_pipeline& gl_pipeline::set_poly_mode(polygon_mode poly_mode) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _poly_mode = poly_mode;
  return *this;
}

gl_pipeline& gl_pipeline::set_poly_width(f32 poly_width) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _poly_width = poly_width;
  return *this;
}

gl_pipeline& gl_pipeline::set_stencil_test(const gl_stencil_test_props& stencil) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _stencil = stencil;
  return *this;
}

gl_pipeline& gl_pipeline::set_depth_test(const gl_depth_test_props& depth) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _depth = depth;
  return *this;
}

gl_pipeline& gl_pipeline::set_blending(const gl_blending_props& blending) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _blending = blending;
  return *this;
}

gl_pipeline& gl_pipeline::set_culling(const gl_culling_props& culling) {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  _culling = culling;
  return *this;
}

gldefs::GLhandle gl_pipeline::program() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _program;
}

gldefs::GLbitfield gl_pipeline::stages() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _stages;
}

auto gl_pipeline::primitive() const -> primitive_mode {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _primitive;
}

auto gl_pipeline::poly_mode() const -> polygon_mode {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _poly_mode;
}

f32 gl_pipeline::poly_width() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _poly_width;
}

const gl_stencil_test_props& gl_pipeline::stencil_test() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _stencil;
}

const gl_depth_test_props& gl_pipeline::depth_test() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _depth;
}

const gl_blending_props& gl_pipeline::blending() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _blending;
}

const gl_culling_props& gl_pipeline::culling() const {
  SHOGLE_ASSERT(_program != GL_NULL_HANDLE, "gl_pipeline use after free");
  return _culling;
}

} // namespace shogle
