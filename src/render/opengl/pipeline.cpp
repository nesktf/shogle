#include "./pipeline.hpp"

namespace ntf {

void gl_pipeline::load(const gl_shader** shaders, uint32 shader_count,
                       const r_attrib_info* attribs, uint32 attrib_count) {
  NTF_ASSERT(!_program_id);

  NTF_ASSERT(shaders && shader_count > 0);
  NTF_ASSERT(attribs && attrib_count > 0);

  r_shader_type shader_flags{r_shader_type::none};
  for (uint32 i = 0; i < shader_count; ++i) {
    const auto& shader = *shaders[i];
    NTF_ASSERT(!+(shader_flags & shader._type), "Detected duplicate shader!!!");
    shader_flags &= shader._type;
  }

  size_t stride{0};
  for (uint32 i = 0; i < attrib_count; ++i) {
    r_attrib_info attrib = attribs[i];
    stride += r_attrib_type_size(attrib.type);
  }

  int succ;
  GLuint id = glCreateProgram();
  for (uint i = 0; i < shader_count; ++i) {
    const auto& shader = *shaders[i];
    glAttachShader(id, shader._id);
  }
  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &succ);
  if (!succ) {
    char log[512];
    glGetShaderInfoLog(shaders[0]->_id, 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_pipeline] Program link failed (id: {}) -> {}", id, log);
    glDeleteProgram(id);
    return;
  }

  _program_id = id;
  if (_attribs.capacity() < attrib_count) {
    _attribs.reserve(attrib_count-_attribs.capacity());
  }
  for (uint32 i = 0; i < attrib_count; ++i) {
    _attribs.emplace_back(attribs[i]);
  }
  _attrib_stride = stride;
  _enabled_shaders = shader_flags;
}

void gl_pipeline::unload() {
  NTF_ASSERT(_program_id);

  glDeleteProgram(_program_id);

  _program_id = 0;
  _enabled_shaders = r_shader_type::none;
  _attribs.clear();
  _attrib_stride = 0;
}

void gl_pipeline::bind(bool with_program) {
  NTF_ASSERT(_program_id);

  for (const auto& attrib : _attribs) {
    const uint32 type_dim = r_attrib_type_dim(attrib.type);
    NTF_ASSERT(type_dim);

    const GLenum gl_underlying_type = gl_attrib_type_underlying_cast(attrib.type);
    NTF_ASSERT(gl_underlying_type);

    // VertexAttribPointers HAVE to be rebound each time a vertex buffer is rebound
    // TODO: Move the bindings and uniforms to the context?
    glVertexAttribPointer(
      attrib.location,
      type_dim,
      gl_underlying_type,
      GL_FALSE, // Don't normalize,
      _attrib_stride,
      reinterpret_cast<void*>(attrib.offset)
    );
    glEnableVertexAttribArray(attrib.location);
  }
  if (with_program) {
    glUseProgram(_program_id);
  }
}

void gl_pipeline::uniform(uint32 location, int32 value) {
  glUniform1i(location, value);
}

void gl_pipeline::uniform(uint32 location, float32 value) {
  glUniform1f(location, value);
}

void gl_pipeline::uniform(uint32 location, float64 value) {
  glUniform1d(location, value);
}

void gl_pipeline::uniform(uint32 location, vec2 value) {
  glUniform2fv(location, 1, glm::value_ptr(value));
}

void gl_pipeline::uniform(uint32 location, vec3 value) {
  glUniform3fv(location, 1, glm::value_ptr(value));
}

void gl_pipeline::uniform(uint32 location, vec4 value) {
  glUniform4fv(location, 1, glm::value_ptr(value));
}

void gl_pipeline::uniform(uint32 location, mat3 value) {
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void gl_pipeline::uniform(uint32 location, mat4 value) {
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

std::optional<uint32> gl_pipeline::uniform_location(std::string_view name) const {
  NTF_ASSERT(_program_id);
  const GLint loc = glGetUniformLocation(_program_id, name.data());
  if (loc < 0) {
    return std::nullopt;
  }
  return loc;
}

} // namespace ntf
