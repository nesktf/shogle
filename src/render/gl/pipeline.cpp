#include "./context_private.hpp"
#include <shogle/render/gl/pipeline.hpp>

namespace shogle {

namespace {

constexpr u32 map_shader(gl_shader::shader_stage stage) noexcept {
  return static_cast<u32>(stage) % gl_shader_builder::MAP_SIZE;
}

// Make sure everything has an unique index
static_assert(map_shader(gl_shader::SHADER_FRAGMENT) == 2);
static_assert(map_shader(gl_shader::SHADER_GEOMETRY) == 3);
static_assert(map_shader(gl_shader::SHADER_VERTEX) == 4);
static_assert(map_shader(gl_shader::SHADER_TESS_EVAL) == 7);
static_assert(map_shader(gl_shader::SHADER_TESS_CTRL) == 8);

} // namespace

gl_shader_builder::gl_shader_builder() noexcept {
  std::memset(_shaders.data(), 0, sizeof(_shaders[0]));
}

gl_shader_builder& gl_shader_builder::add_shader(const gl_shader& shader) {
  NTF_ASSERT(shader.stage() != gl_shader::SHADER_COMPUTE,
             "Can't use compute shaders in a graphics pipeline");
  _shaders[map_shader(shader.stage())] = shader.id();
  return *this;
}

gldefs::GLhandle gl_shader_builder::get_shader(gl_shader::shader_stage stage) {
  return _shaders[map_shader(stage)];
}

ntf::optional<gl_graphics_shader_set> gl_shader_builder::build() {
  const auto vert = _shaders[map_shader(gl_shader::SHADER_VERTEX)];
  if (!vert) {
    return {ntf::nullopt};
  }
  const auto frag = _shaders[map_shader(gl_shader::SHADER_FRAGMENT)];
  if (!frag) {
    return {ntf::nullopt};
  }
  gl_graphics_shader_set set;
  set.shader_count = 0;
  const auto add_shader = [&](gldefs::GLhandle shader) {
    NTF_ASSERT(set.shader_count < set.shaders.size());
    set.shaders[set.shader_count++] = shader;
  };
  add_shader(vert);
  add_shader(frag);
  if (auto geom = _shaders[map_shader(gl_shader::SHADER_GEOMETRY)]; geom) {
    add_shader(geom);
  }

  // For tesselation, the evaluation stage is always required
  // and the control stage is optional
  const auto eval = _shaders[map_shader(gl_shader::SHADER_TESS_EVAL)];
  const auto ctrl = _shaders[map_shader(gl_shader::SHADER_TESS_CTRL)];
  if (ctrl && !eval) {
    return {ntf::nullopt};
  }
  if (eval) {
    add_shader(eval);
  }
  if (ctrl) {
    add_shader(ctrl);
  }

  return {ntf::in_place, std::move(set)};
}

fn gl_shader::create(gl_context& gl, std::string_view src, shader_stage stage)
  -> gl_s_expect<gl_shader> {
  NTF_UNUSED(gl);
  static constexpr auto shader_types = std::to_array({
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
  });
  NTF_ASSERT(stage < SHADER_COUNT);

  GLenum id = glCreateShader(shader_types[stage]);
  NTF_ASSERT(id != NULL_BINDING);

  const char* src_data = src.data();
  const GLint len = static_cast<GLint>(src.size());
  GL_ASSERT(glShaderSource(id, 1, &src_data, &len));
  glCompileShader(id);

  int succ;
  GL_ASSERT(glGetShaderiv(id, GL_COMPILE_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_ASSERT(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &err_len));
    char log_buffer[1024] = {0};
    GL_ASSERT(glGetShaderInfoLog(id, 1024, &err_len, &log_buffer[0]));
    GL_ASSERT(glDeleteShader(id));
    std::string_view buffer_view(log_buffer, std::min(err_len, 1024));
    return {ntf::unexpect, fmt::format("Shader compilation failed: {}", buffer_view)};
  }
  return {ntf::in_place, id, stage};
}

fn gl_shader::destroy(gl_context& gl) -> void {
  NTF_UNUSED(gl);
  NTF_ASSERT(_id != NULL_BINDING, "gl_shader use after free");
  GL_ASSERT(glDeleteShader(_id));
  _id = NULL_BINDING;
}

fn gl_shader::id() const -> gl_handle {
  NTF_ASSERT(_id != NULL_BINDING, "gl_shader use after free");
  return _id;
}

fn gl_shader::stage() const -> GLenum {
  NTF_ASSERT(_id != NULL_BINDING, "gl_shader use after free");
  return _stage;
}

gl_shader::gl_shader(gl_handle id, shader_stage stage) : _id(id), _stage(stage) {}

fn gl_pipeline::create(gl_context& gl, span<const gl_attribute_binding> attributes,
                       gl_shader::shader_map shaders, GLenum primitive, GLenum polymode)
  -> gl_s_expect<gl_pipeline> {
  NTF_UNUSED(gl);
  if (attributes.empty() || attributes.size() > MAX_ATTRIBUTE_BINDINGS) {
    return {ntf::unexpect, "Invalid attribute binding count"};
  }
  if (shaders[gl_shader::SHADER_VERTEX] == gl_shader::NULL_BINDING) {
    return {ntf::unexpect, "No vertex shader provided"};
  }
  if (shaders[gl_shader::SHADER_FRAGMENT] == gl_shader::NULL_BINDING) {
    return {ntf::unexpect, "No fragment shader provided"};
  }

  GLuint program = glCreateProgram();
  NTF_ASSERT(program != NULL_BINDING);

  for (const gl_handle shader : shaders) {
    GL_ASSERT(glAttachShader(program, shader));
  }
  glLinkProgram(program);

  int succ;
  GL_ASSERT(glGetProgramiv(program, GL_LINK_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_ASSERT(glGetShaderiv(shaders[gl_shader::SHADER_VERTEX], GL_INFO_LOG_LENGTH, &err_len));
    char log_buffer[1024] = {0};
    GL_ASSERT(
      glGetShaderInfoLog(shaders[gl_shader::SHADER_VERTEX], 1024, &err_len, &log_buffer[0]));
    GL_ASSERT(glDeleteProgram(program));
    std::string_view buffer_view(log_buffer, std::min(err_len, 1024));
    return {ntf::unexpect, fmt::format("Program linking failed: {}", buffer_view)};
  }

  for (const gl_handle shader : shaders) {
    GL_ASSERT(glDetachShader(program, shader));
  }

  attribute_bindings binds{};
  for (const auto& attribute : attributes) {
    binds.bindings[binds.count++] = attribute;
  }
  GLuint vao;
  GL_ASSERT(glCreateVertexArrays(1, &vao));
  return {ntf::in_place, program, primitive, polymode, vao, binds};
}

fn gl_pipeline::destroy(gl_context& gl) -> void {
  NTF_UNUSED(gl);
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  GL_ASSERT(glDeleteVertexArrays(1, &_vao));
  GL_ASSERT(glDeleteProgram(_program));
  _program = NULL_BINDING;
}

fn gl_pipeline::uniform_location(const char* name) -> ntf::optional<u32> {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  GLint loc = glGetUniformLocation(_program, name);
  if (loc == -1) {
    return {ntf::nullopt};
  }
  return {ntf::in_place, static_cast<u32>(loc)};
}

fn gl_pipeline::_query_uniform_index(u32 idx) -> uniform_props {
  GLint size;
  GLsizei len;
  uniform_props props;
  GL_ASSERT(glGetActiveUniform(_program, static_cast<GLuint>(idx), MAX_UNIFORM_NAME_SIZE, &len,
                               &size, &props.type, props.name));
  props.size = static_cast<size_t>(size);
  props.name_len = static_cast<u32>(len);
  return props;
}

fn gl_pipeline::reset_props() -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _poly_width = 1.f;
  _stencil.enable = false;
  _depth.enable = false;
  _blending.enable = false;
  _culling.enable = false;
  return *this;
}

fn gl_pipeline::set_poly_mode(GLenum poly_mode) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _poly_mode = poly_mode;
  return *this;
}

fn gl_pipeline::set_poly_width(f32 poly_width) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _poly_width = poly_width;
  return *this;
}

fn gl_pipeline::set_primitive(GLenum primitive) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _primitive = primitive;
  return *this;
}

fn gl_pipeline::set_stencil_test(const stencil_test_props& stencil) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _stencil = stencil;
  return *this;
}

fn gl_pipeline::set_depth_test(const depth_test_props& depth) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _depth = depth;
  return *this;
}

fn gl_pipeline::set_blending(const blending_props& blending) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _blending = blending;
  return *this;
}

fn gl_pipeline::set_culling(const culling_props& culling) -> gl_pipeline& {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  _culling = culling;
  return *this;
}

fn gl_pipeline::program() const -> gl_handle {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _program;
}

fn gl_pipeline::vao() const -> gl_handle {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _vao;
}

fn gl_pipeline::attributes() const -> span<const gl_attribute_binding> {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return {_attributes.bindings.data(), _attributes.count};
}

fn gl_pipeline::primitive() const -> GLenum {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _primitive;
}

fn gl_pipeline::poly_mode() const -> GLenum {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _poly_mode;
}

fn gl_pipeline::poly_width() const -> f32 {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _poly_width;
}

fn gl_pipeline::stencil_test() const -> stencil_test_props {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _stencil;
}

fn gl_pipeline::depth_test() const -> depth_test_props {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _depth;
}

fn gl_pipeline::blending() const -> blending_props {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _blending;
}

fn gl_pipeline::culling() const -> culling_props {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  return _culling;
}

fn gl_pipeline::uniform_count() const -> u32 {
  NTF_ASSERT(_program != NULL_BINDING, "gl_pipeline use after free");
  GLint count;
  GL_ASSERT(glGetProgramiv(_program, GL_ACTIVE_UNIFORMS, &count));
  return static_cast<u32>(count);
}

gl_pipeline::gl_pipeline(gl_handle program, GLenum primitive, GLenum poly_mode, gl_handle vao,
                         attribute_bindings attributes) :
    _attributes(attributes), _program(program), _vao(vao), _primitive(primitive),
    _poly_mode(poly_mode) {
  reset_props();
}

} // namespace shogle
