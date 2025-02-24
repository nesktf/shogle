#include "./opengl.hpp"

#ifdef SHOGLE_GL_DISABLE_ASSERTIONS
#define GL_CALL(fun) fun
#else
#define GL_CALL(fun) \
do { \
  fun; \
  GLenum glerr = gl_check_error(__FILE__, __LINE__); \
  NTF_ASSERT(glerr == 0, "GL ERROR: {}", glerr); \
} while(0)
#endif

#define GL_CHECK(fun) \
[&]() { \
  fun; \
  return gl_check_error(__FILE__, __LINE__); \
}()

static GLenum gl_check_error(const char* file, int line) noexcept {
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
    SHOGLE_LOG(error, "[ntf::gl_check_error] GL ERROR ({}) | \"{}\":{} -> {}",
               err, file, line, err_str);
  }
  return out;
}

namespace ntf {

gl_state::gl_state(gl_context& ctx) noexcept :
  _ctx{ctx},
  // _tex_limits{0, 0, 0},
  _bound_vao{0},
  _bound_program{0},
  _active_tex{0} {
  std::memset(_bound_buffers, NULL_BINDING, BUFFER_TYPE_COUNT*sizeof(GLuint));
  std::memset(_bound_fbos, DEFAULT_FBO, FBO_BIND_COUNT*sizeof(GLuint));
}

void gl_state::init(const init_data_t& data) noexcept {
  GLint max_tex;
  GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tex));
  _bound_texs.resize(max_tex, std::make_pair(NULL_BINDING, GL_TEXTURE_2D));

  GLint max_tex_lay;
  GL_CALL(glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_lay));
  _tex_limits.max_layers = max_tex_lay;

  GLint max_tex_dim;
  GL_CALL(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_dim));
  _tex_limits.max_dim = max_tex_dim;

  GLint max_tex_dim3d;
  GL_CALL(glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max_tex_dim3d));
  _tex_limits.max_dim3d = max_tex_dim3d;

  // GLint max_fbo_attach;
  // glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_fbo_attach);
  // _fbo_max_attachments = max_fbo_attach;

  // State cleanup
  GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, DEFAULT_FBO));
  GL_CALL(glUseProgram(NULL_BINDING));
  GL_CALL(glBindVertexArray(NULL_BINDING));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, NULL_BINDING));
  GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL_BINDING));
  GL_CALL(glBindBuffer(GL_UNIFORM_BUFFER, NULL_BINDING));
  GL_CALL(glBindBuffer(GL_TEXTURE_BUFFER, NULL_BINDING));
  GL_CALL(glBindBuffer(GL_SHADER_STORAGE_BUFFER, NULL_BINDING));
  for (GLint i = max_tex-1; i >= 0; --i) {
    // Do it in reverse to end up with GL_TEXTURE0 active
    GL_CALL(glActiveTexture(GL_TEXTURE0+i));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, NULL_BINDING));
  }

  if (data.dbg) {
    GL_CALL(glEnable(GL_DEBUG_OUTPUT));
    GL_CALL(glDebugMessageCallback(data.dbg, &_ctx));
  }
  GL_CALL(glEnable(GL_DEPTH_TEST)); // (?)
}

GLenum gl_state::buffer_type_cast(r_buffer_type type) noexcept {
  switch(type) {
    case r_buffer_type::vertex:         return GL_ARRAY_BUFFER;
    case r_buffer_type::index:          return GL_ELEMENT_ARRAY_BUFFER;
    case r_buffer_type::uniform:        return GL_UNIFORM_BUFFER;
    case r_buffer_type::texel:          return GL_TEXTURE_BUFFER;
    case r_buffer_type::shader_storage: return GL_SHADER_STORAGE_BUFFER;
  };

  NTF_UNREACHABLE();
}

GLenum& gl_state::_buffer_pos(GLenum type) {
  switch (type) {
    case GL_ARRAY_BUFFER:           return _bound_buffers[BUFFER_TYPE_VERTEX];
    case GL_ELEMENT_ARRAY_BUFFER:   return _bound_buffers[BUFFER_TYPE_INDEX];
    case GL_TEXTURE_BUFFER:         return _bound_buffers[BUFFER_TYPE_TEXEL];
    case GL_UNIFORM_BUFFER:         return _bound_buffers[BUFFER_TYPE_UNIFORM];
    case GL_SHADER_STORAGE_BUFFER:  return _bound_buffers[BUFFER_TYPE_SHADER];
  }

  NTF_UNREACHABLE();
}

auto gl_state::create_buffer(r_buffer_type type, r_buffer_flag flags, size_t size,
                             weak_ref<r_buffer_data> data) -> buffer_t {
  NTF_ASSERT(data);
  NTF_ASSERT(data->data);
  NTF_ASSERT(data->offset+data->size <= size);
  GLuint id;
  GL_CALL(glGenBuffers(1, &id));
  const GLenum gltype = buffer_type_cast(type);

  _buffer_pos(gltype) = id;

  GLbitfield glflags = 
    GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT | GL_MAP_PERSISTENT_BIT |
    (+(flags & r_buffer_flag::dynamic_storage) ? GL_DYNAMIC_STORAGE_BIT : 0);

  GLuint last = _buffer_pos(gltype);
  GL_CALL(glBindBuffer(gltype, id));
  const void* data_ptr = 
    reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(data->data) + data->offset);
  if (auto err = GL_CHECK(glBufferStorage(gltype, size, data_ptr, glflags)); err != GL_NO_ERROR) {
    GL_CALL(glBindBuffer(gltype, last));
    GL_CALL(glDeleteBuffers(1, &id));
    throw error<>::format({"ntf::gl_state] Failed to create buffer with size {}"}, size);
  }
  _buffer_pos(gltype) = id;

  buffer_t buff;
  buff.id = id;
  buff.size = size;
  buff.type = gltype;
  buff.flags = glflags;
  return buff;
}

void gl_state::destroy_buffer(const buffer_t& buffer) noexcept {
  NTF_ASSERT(buffer.id);
  GLuint id = buffer.id;
  GLenum& pos = _buffer_pos(buffer.type);
  if (pos == id) {
    GL_CALL(glBindBuffer(buffer.type, NULL_BINDING));
    pos = NULL_BINDING;
  }
  GL_CALL(glDeleteBuffers(1, &id));
}

bool gl_state::bind_buffer(GLuint id, GLenum type) noexcept {
  GLenum& pos = _buffer_pos(type);
  if (pos == id) {
    return false;
  }
  GL_CALL(glBindBuffer(type, id));
  pos = id;
  return true;
}

void gl_state::update_buffer(const buffer_t& buffer, const void* data,
                             size_t size, size_t off) noexcept {
  NTF_ASSERT(buffer.flags & GL_DYNAMIC_STORAGE_BIT);
  NTF_ASSERT(size+off <= buffer.size);

  bind_buffer(buffer.id, buffer.type);
  GL_CALL(glBufferSubData(buffer.type, off, size, data));
}

auto gl_state::create_vao() noexcept -> vao_t {
  GLuint id;
  GL_CALL(glGenVertexArrays(1, &id));
  vao_t vao;
  vao.id = id;
  return vao;
}

void gl_state::bind_vao(GLuint id) noexcept {
  if (_bound_vao == id) {
    return;
  }
  GL_CALL(glBindVertexArray(id));
  _bound_vao = id;
}

void gl_state::destroy_vao(const vao_t& vao) noexcept {
  NTF_ASSERT(vao.id);
  GLuint id = vao.id;
  if (_bound_vao == id) {
    _bound_vao = NULL_BINDING;
    GL_CALL(glBindVertexArray(NULL_BINDING));
  }
  GL_CALL(glDeleteVertexArrays(1, &id));
}

GLenum gl_state::shader_type_cast(r_shader_type type) noexcept {
  switch (type) {
    case r_shader_type::vertex:               return GL_VERTEX_SHADER;
    case r_shader_type::fragment:             return GL_FRAGMENT_SHADER;
    case r_shader_type::geometry:             return GL_GEOMETRY_SHADER;
    case r_shader_type::tesselation_eval:     return GL_TESS_EVALUATION_SHADER;
    case r_shader_type::tesselation_control:  return GL_TESS_CONTROL_SHADER;

    case r_shader_type::compute: break;
  }

  NTF_UNREACHABLE();
}

auto gl_state::create_shader(r_shader_type type, std::string_view src) -> shader_t {
  const GLenum gltype = shader_type_cast(type);
  GLenum id;
  GL_CALL(id = glCreateShader(gltype));

  const char* src_data = src.data();
  const GLint len = src.size();
  GL_CALL(glShaderSource(id, 1, &src_data, &len));
  GL_CALL(glCompileShader(id));

  int succ;
  GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_CALL(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &err_len));
    std::string log;
    log.resize(err_len);

    GL_CALL(glGetShaderInfoLog(id, 1024, &err_len, log.data()));
    GL_CALL(glDeleteShader(id));
    throw error<>::format({"[ntf::gl_state] Failed to compile shader: {}"}, log);
  }

  shader_t shader;
  shader.id = id;
  shader.type = gltype;
  return shader;
}

void gl_state::destroy_shader(const shader_t& shader) noexcept {
  NTF_ASSERT(shader.id);
  GL_CALL(glDeleteShader(shader.id));
}

auto gl_state::create_program(shader_t const* const* shaders, uint32 count,
                              r_primitive primitive) -> program_t {
  GLuint id;
  GL_CALL(id = glCreateProgram());
  for (uint32 i = 0; i < count; ++i) {
    // TODO: Ensure vertex and fragment exist
    // TODO: Check for duplicate shader types
    GL_CALL(glAttachShader(id, shaders[i]->id));
  }
  GL_CALL(glLinkProgram(id));

  int succ;
  GL_CALL(glGetProgramiv(id, GL_LINK_STATUS, &succ));
  if (!succ) {
    GLint err_len = 0; // includes null terminator
    GL_CALL(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &err_len));
    std::string log;
    log.resize(err_len);

    GL_CALL(glGetShaderInfoLog(shaders[0]->id, 1024, &err_len, log.data()));
    GL_CALL(glDeleteProgram(id));
    throw error<>::format({"[ntf::gl_state] Failed to link program: {}"}, log);
  }

  for (uint32 i = 0; i < count; ++i) {
    GL_CALL(glDetachShader(id, shaders[i]->id));
  }

  program_t prog;
  prog.id = id;
  prog.primitive = primitive_cast(primitive);
  return prog;
}

void gl_state::destroy_program(const program_t& prog) noexcept {
  NTF_ASSERT(prog.id);
  if (_bound_program == prog.id) {
    _bound_program = NULL_BINDING;
    GL_CALL(glUseProgram(NULL_BINDING));
  }
  GL_CALL(glDeleteProgram(prog.id));
}

bool gl_state::bind_program(GLuint id) noexcept {
  if (_bound_program == id) {
    return false;
  }
  GL_CALL(glUseProgram(id));
  _bound_program = id;
  return true;
}

void gl_state::push_uniform(uint32 loc, r_attrib_type type, const void* data) noexcept {
  NTF_ASSERT(data);
  switch (type) {
    case r_attrib_type::f32: {
      GL_CALL(glUniform1f(
        loc, *reinterpret_cast<const float32*>(data)
      ));
      break;
    }
    case r_attrib_type::vec2: {
      GL_CALL(glUniform2fv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const vec2*>(data))
      ));
      break;
    }
    case r_attrib_type::vec3: {
      GL_CALL(glUniform3fv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const vec3*>(data))
      ));
      break;
    }
    case r_attrib_type::vec4: {
      GL_CALL(glUniform4fv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const vec4*>(data))
      ));
      break;
    }
    case r_attrib_type::mat3: {
      GL_CALL(glUniformMatrix3fv(
        loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const mat3*>(data))
      ));
      break;
    } 
    case r_attrib_type::mat4: {
      GL_CALL(glUniformMatrix4fv(
        loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const mat4*>(data))
      ));
      break;
    }

    case r_attrib_type::f64: {
      GL_CALL(glUniform1d(
        loc, *reinterpret_cast<const float64*>(data)
      ));
      break;
    }
    case r_attrib_type::dvec2: {
      GL_CALL(glUniform2dv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const dvec2*>(data))
      ));
      break;
    }
    case r_attrib_type::dvec3: {
      GL_CALL(glUniform3dv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const dvec3*>(data))
      ));
      break;
    }
    case r_attrib_type::dvec4: {
      GL_CALL(glUniform4dv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const dvec4*>(data))
      ));
      break;
    }
    case r_attrib_type::dmat3: {
      GL_CALL(glUniformMatrix3dv(
        loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const dmat3*>(data))
      ));
      break;
    } 
    case r_attrib_type::dmat4: {
      GL_CALL(glUniformMatrix4dv(
        loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const dmat4*>(data))
      ));
      break;
    }

    case r_attrib_type::i32: {
      GL_CALL(glUniform1i(
        loc, *reinterpret_cast<const int32*>(data)
      ));
      break;
    }
    case r_attrib_type::ivec2: {
      GL_CALL(glUniform2iv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const ivec2*>(data))
      ));
      break;
    }
    case r_attrib_type::ivec3: {
      GL_CALL(glUniform3iv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const ivec3*>(data))
      ));
      break;
    }
    case r_attrib_type::ivec4: {
      GL_CALL(glUniform4iv(
        loc, 1, glm::value_ptr(*reinterpret_cast<const ivec4*>(data))
      ));
      break;
    }

    default: {
      NTF_UNREACHABLE();
    }
  };
}

void gl_state::query_program_uniforms(const program_t& prog, r_context::uniform_map& unif) {
  NTF_ASSERT(unif.empty());
  int count;
  GL_CALL(glGetProgramiv(prog.id, GL_ACTIVE_UNIFORMS, &count));
  NTF_ASSERT(count);
  for (int i = 0; i < count; ++i) {
    GLint size;
    GLenum type;
    GLchar name[128];
    GLsizei len;
    GL_CALL(glGetActiveUniform(prog.id, static_cast<GLuint>(i), 128, &len, &size, &type, name));
    auto [_, emplaced] = unif.try_emplace(
      std::string{name, static_cast<size_t>(len)}, r_uniform{static_cast<uint32>(i)}
    );
    NTF_ASSERT(emplaced);
  }
}

// r_uniform gl_state::uniform_location(GLuint program, std::string_view name) noexcept {
//   NTF_ASSERT(program);
//   // TODO: Check for null termination?
//   const GLint loc = glGetUniformLocation(program, name.data());
//   if (loc < 0) {
//     return r_uniform{};
//   }
//   return r_uniform{static_cast<r_handle_value>(loc)};
// }

GLenum gl_state::texture_type_cast(r_texture_type type, bool is_array) noexcept {
  switch (type) {
    case r_texture_type::texture1d:       return is_array ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
    case r_texture_type::texture2d:       return is_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
    case r_texture_type::texture3d:       return GL_TEXTURE_3D;
    case r_texture_type::cubemap:         return GL_TEXTURE_CUBE_MAP;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::r8n:       return GL_R8_SNORM;
    case r_texture_format::r8u:       return GL_R8UI;
    case r_texture_format::r8i:       return GL_R8I;
    case r_texture_format::r16u:      return GL_R16UI;
    case r_texture_format::r16i:      return GL_R16I;
    case r_texture_format::r16f:      return GL_R16F;
    case r_texture_format::r32u:      return GL_R32UI;
    case r_texture_format::r32i:      return GL_R32I;
    case r_texture_format::r32f:      return GL_R32F;

    case r_texture_format::rg8n:      return GL_RG8_SNORM;
    case r_texture_format::rg8u:      return GL_RG8UI;
    case r_texture_format::rg8i:      return GL_RG8I;
    case r_texture_format::rg16u:     return GL_RG16UI;
    case r_texture_format::rg16i:     return GL_RG16I;
    case r_texture_format::rg16f:     return GL_RG16F;
    case r_texture_format::rg32u:     return GL_RG32UI;
    case r_texture_format::rg32i:     return GL_RG32I;
    case r_texture_format::rg32f:     return GL_RG32F;

    case r_texture_format::rgb8n:     return GL_RGB8_SNORM;
    case r_texture_format::rgb8u:     return GL_RGB8UI;
    case r_texture_format::rgb8i:     return GL_RGB8I;
    case r_texture_format::rgb16u:    return GL_RGB16UI;
    case r_texture_format::rgb16i:    return GL_RGB16I;
    case r_texture_format::rgb16f:    return GL_RGB16F;
    case r_texture_format::rgb32u:    return GL_RGB32UI;
    case r_texture_format::rgb32i:    return GL_RGB32I;
    case r_texture_format::rgb32f:    return GL_RGB32F;

    case r_texture_format::rgba8n:     return GL_RGBA8_SNORM;
    case r_texture_format::rgba8u:    return GL_RGBA8UI;
    case r_texture_format::rgba8i:    return GL_RGBA8UI;
    case r_texture_format::rgba16u:   return GL_RGBA16UI;
    case r_texture_format::rgba16i:   return GL_RGBA16I;
    case r_texture_format::rgba16f:   return GL_RGBA16F;
    case r_texture_format::rgba32u:   return GL_RGBA32UI;
    case r_texture_format::rgba32i:   return GL_RGBA32I;
    case r_texture_format::rgba32f:   return GL_RGBA32F;

    case r_texture_format::srgb8u:    return GL_SRGB8;
    case r_texture_format::srgba8u:   return GL_SRGB8_ALPHA8;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_symbolic_cast(r_texture_format format) noexcept {

  switch (format) {
    case r_texture_format::r8n:       [[fallthrough]];
    case r_texture_format::r8u:       [[fallthrough]];
    case r_texture_format::r8i:       [[fallthrough]];
    case r_texture_format::r16u:      [[fallthrough]];
    case r_texture_format::r16i:      [[fallthrough]];
    case r_texture_format::r16f:      [[fallthrough]];
    case r_texture_format::r32u:      [[fallthrough]];
    case r_texture_format::r32i:      [[fallthrough]];
    case r_texture_format::r32f:      return GL_RED;

    case r_texture_format::rg8n:      [[fallthrough]];
    case r_texture_format::rg8u:      [[fallthrough]];
    case r_texture_format::rg8i:      [[fallthrough]];
    case r_texture_format::rg16u:     [[fallthrough]];
    case r_texture_format::rg16i:     [[fallthrough]];
    case r_texture_format::rg16f:     [[fallthrough]];
    case r_texture_format::rg32u:     [[fallthrough]];
    case r_texture_format::rg32i:     [[fallthrough]];
    case r_texture_format::rg32f:     return GL_RG;

    case r_texture_format::rgb8n:     [[fallthrough]];
    case r_texture_format::rgb8u:     [[fallthrough]];
    case r_texture_format::rgb8i:     [[fallthrough]];
    case r_texture_format::rgb16u:    [[fallthrough]];
    case r_texture_format::rgb16i:    [[fallthrough]];
    case r_texture_format::rgb16f:    [[fallthrough]];
    case r_texture_format::rgb32u:    [[fallthrough]];
    case r_texture_format::rgb32i:    [[fallthrough]];
    case r_texture_format::rgb32f:    return GL_RGB;

    case r_texture_format::rgba8n:    [[fallthrough]];
    case r_texture_format::rgba8u:    [[fallthrough]];
    case r_texture_format::rgba8i:    [[fallthrough]];
    case r_texture_format::rgba16u:   [[fallthrough]];
    case r_texture_format::rgba16i:   [[fallthrough]];
    case r_texture_format::rgba16f:   [[fallthrough]];
    case r_texture_format::rgba32u:   [[fallthrough]];
    case r_texture_format::rgba32i:   [[fallthrough]];
    case r_texture_format::rgba32f:   return GL_RGBA;

    case r_texture_format::srgb8u:    [[fallthrough]];
    case r_texture_format::srgba8u:   return GL_SRGB;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_underlying_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::r8u:       [[fallthrough]];
    case r_texture_format::r8n:       [[fallthrough]];
    case r_texture_format::rg8u:      [[fallthrough]];
    case r_texture_format::rg8n:      [[fallthrough]];
    case r_texture_format::rgb8u:     [[fallthrough]];
    case r_texture_format::rgb8n:     [[fallthrough]];
    case r_texture_format::rgba8u:    [[fallthrough]];
    case r_texture_format::rgba8n:    [[fallthrough]];
    case r_texture_format::srgb8u:    [[fallthrough]];
    case r_texture_format::srgba8u:   return GL_UNSIGNED_BYTE;

    case r_texture_format::r8i:       [[fallthrough]];
    case r_texture_format::rg8i:      [[fallthrough]];
    case r_texture_format::rgb8i:     [[fallthrough]];
    case r_texture_format::rgba8i:    return GL_BYTE;

    case r_texture_format::r16u:      [[fallthrough]];
    case r_texture_format::rg16u:     [[fallthrough]];
    case r_texture_format::rgb16u:    [[fallthrough]];
    case r_texture_format::rgba16u:   return GL_UNSIGNED_SHORT;

    case r_texture_format::r16i:      [[fallthrough]];
    case r_texture_format::rg16i:     [[fallthrough]];
    case r_texture_format::rgb16i:    [[fallthrough]];
    case r_texture_format::rgba16i:   return GL_SHORT;

    case r_texture_format::r16f:      [[fallthrough]];
    case r_texture_format::rg16f:     [[fallthrough]];
    case r_texture_format::rgb16f:    [[fallthrough]];
    case r_texture_format::rgba16f:   return GL_HALF_FLOAT;

    case r_texture_format::r32u:      [[fallthrough]];
    case r_texture_format::rg32u:     [[fallthrough]];
    case r_texture_format::rgb32u:    [[fallthrough]];
    case r_texture_format::rgba32u:   return GL_UNSIGNED_INT;

    case r_texture_format::r32i:      [[fallthrough]];
    case r_texture_format::rg32i:     [[fallthrough]];
    case r_texture_format::rgb32i:    [[fallthrough]];
    case r_texture_format::rgba32i:   return GL_INT;

    case r_texture_format::r32f:      [[fallthrough]];
    case r_texture_format::rg32f:     [[fallthrough]];
    case r_texture_format::rgb32f:    [[fallthrough]];
    case r_texture_format::rgba32f:   return GL_FLOAT;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_sampler_cast(r_texture_sampler sampler, bool mipmaps) noexcept {
  // TODO: Add cases for GL_LINEAR_MIPMAP_NEAREST and GL_NEAREST_MIPMAP_LINEAR?
  switch (sampler) {
    case r_texture_sampler::nearest:  return mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case r_texture_sampler::linear:   return mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
  }

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_addressing_cast(r_texture_address address) noexcept {
  switch (address) {
    case r_texture_address::clamp_border:         return GL_CLAMP_TO_BORDER;
    case r_texture_address::repeat:               return GL_REPEAT;
    case r_texture_address::repeat_mirrored:      return GL_MIRRORED_REPEAT;
    case r_texture_address::clamp_edge:           return GL_CLAMP_TO_EDGE;
    case r_texture_address::clamp_edge_mirrored:  return GL_MIRROR_CLAMP_TO_EDGE;
  };

  NTF_UNREACHABLE();
}

auto gl_state::create_texture(r_texture_type type, r_texture_format format,
                              r_texture_sampler sampler, r_texture_address addressing,
                              uvec3 extent, uint32 layers, uint32 levels) -> texture_t {
  NTF_ASSERT(levels <= MAX_TEXTURE_LEVEL && levels > 0);

  const GLenum gltype = texture_type_cast(type, (layers > 1));
  const GLenum glformat = texture_format_cast(format);

  GLuint id;
  GL_CALL(glGenTextures(1, &id));

  auto& [active_id, active_type] = _bound_texs[_active_tex];
  GL_CALL(glBindTexture(gltype, id));
  active_id = id;
  active_type = gltype;

  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      NTF_ASSERT(layers > 0);
      GL_CALL(glTexStorage2D(gltype, levels, glformat, extent.x, layers));
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      GL_CALL(glTexStorage1D(gltype, levels, glformat, extent.x));
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      NTF_ASSERT(extent.y > 0 && extent.y <= _tex_limits.max_dim);
      NTF_ASSERT(layers > 0);
      GL_CALL(glTexStorage3D(gltype, levels, glformat, extent.x, extent.y, layers));
      break;
    }

    case GL_TEXTURE_CUBE_MAP:
      NTF_ASSERT(extent.x == extent.y);
      [[fallthrough]];
    case GL_TEXTURE_2D: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim);
      NTF_ASSERT(extent.y > 0 && extent.y <= _tex_limits.max_dim);
      GL_CALL(glTexStorage2D(gltype, levels, glformat, extent.x, extent.y));
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(extent.x > 0 && extent.x <= _tex_limits.max_dim3d);
      NTF_ASSERT(extent.y > 0 && extent.y <= _tex_limits.max_dim3d);
      NTF_ASSERT(extent.z > 0 && extent.z <= _tex_limits.max_dim3d);
      GL_CALL(glTexStorage3D(gltype, levels, glformat, extent.x, extent.y, extent.z));
      break;
    };

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };

  const GLenum glsamplermin = texture_sampler_cast(sampler, (levels > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  GL_CALL(glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glsamplermag));
  GL_CALL(glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glsamplermin));

  const GLenum gladdress = texture_addressing_cast(addressing);
  GL_CALL(glTexParameteri(gltype, GL_TEXTURE_WRAP_S, gladdress)); // U
  if (gltype != GL_TEXTURE_1D || gltype != GL_TEXTURE_1D_ARRAY) {
    GL_CALL(glTexParameteri(gltype, GL_TEXTURE_WRAP_T, gladdress)); // V
    if (gltype == GL_TEXTURE_3D || gltype == GL_TEXTURE_CUBE_MAP) {
      GL_CALL(glTexParameteri(gltype, GL_TEXTURE_WRAP_R, gladdress)); // W (?)
    }
  }

  texture_t tex;
  tex.id = id;
  tex.type = gltype;
  tex.format = glformat;
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
  tex.addressing = gladdress;
  tex.extent = extent;
  tex.layers = layers;
  tex.levels = levels;

  // if (data) {
  //   update_texture_data(tex, data, format, uvec3{0, 0, 0}, 0, 0, genmips);
  // }

  return tex;
}

void gl_state::destroy_texture(const texture_t& tex) noexcept {
  NTF_ASSERT(tex.id);
  auto& [id, type] = _bound_texs[_active_tex];
  if (id == tex.id) {
    GL_CALL(glBindTexture(type, NULL_BINDING));
    id = NULL_BINDING;
    type = NULL_BINDING;
  }
  GLuint texid = tex.id;
  GL_CALL(glDeleteTextures(1, &texid));
}

void gl_state::bind_texture(GLuint id, GLenum type, uint32 index) noexcept {
  NTF_ASSERT(type);
  NTF_ASSERT(index < _bound_texs.size());
  auto& [bound, bound_type] = _bound_texs[index];
  if (bound == id) {
    return;
  }
  GL_CALL(glActiveTexture(GL_TEXTURE0+index));
  GL_CALL(glBindTexture(type, id));
  bound = id;
  bound_type = type;
}

void gl_state::update_texture_data(const texture_t& tex, const void* data, 
                                   r_texture_format format,
                                   uvec3 offset, uint32 layer,
                                   uint32 level) noexcept {
  NTF_ASSERT(tex.id);
  NTF_ASSERT(data);
  NTF_ASSERT(level < tex.levels);
  // const GLenum data_format = texture_format_cast(format);
  const GLenum data_format = texture_format_symbolic_cast(format);
  const GLenum data_format_type = texture_format_underlying_cast(format);

  NTF_ASSERT(data_format != GL_SRGB, "Can't use SRGB with glTexSubImage");

  bind_texture(tex.id, tex.type, _active_tex);
  switch (tex.type) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(layer < tex.layers);
      GL_CALL(glTexSubImage2D(tex.type, level,
                              offset.x, layer,
                              tex.extent.x, tex.layers,
                              data_format, data_format_type, data));
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      GL_CALL(glTexSubImage1D(tex.type, level,
                              offset.x, tex.extent.x,
                              data_format, data_format_type, data));
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(layer < tex.layers);
      GL_CALL(glTexSubImage3D(tex.type, level,
                              offset.x, offset.y, layer, 
                              tex.extent.x, tex.extent.y, tex.layers,
                              data_format, data_format_type, data));
      break;
    }

    case GL_TEXTURE_2D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      GL_CALL(glTexSubImage2D(tex.type, level,
                              offset.x, offset.y,
                              tex.extent.x, tex.extent.y,
                              data_format, data_format_type, data));
      break;
    }
    
    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(layer < 6);
      GL_CALL(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+layer, level,
                              offset.x, offset.y,
                              tex.extent.x, tex.extent.y,
                              data_format, data_format_type, data));
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(offset.x < tex.extent.x);
      NTF_ASSERT(offset.y < tex.extent.y);
      NTF_ASSERT(offset.z < tex.extent.z);
      GL_CALL(glTexSubImage3D(tex.type, level,
                              offset.x, offset.y, offset.z,
                              tex.extent.x, tex.extent.y, tex.extent.z,
                              data_format, data_format_type, data));
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
}

void gl_state::update_texture_sampler(texture_t& tex, r_texture_sampler sampler) noexcept {
  NTF_ASSERT(tex.id);
  const GLenum glsamplermin = texture_sampler_cast(sampler, (tex.levels > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  if (tex.sampler[0] == glsamplermin && tex.sampler[1] == glsamplermag) {
    return;
  }

  bind_texture(tex.id, tex.type, _active_tex);
  GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_MAG_FILTER, glsamplermag));
  GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_MIN_FILTER, glsamplermin));
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
}

void gl_state::update_texture_addressing(texture_t& tex, r_texture_address addressing) noexcept {
  NTF_ASSERT(tex.id);
  const GLenum gladdress = texture_addressing_cast(addressing);
  if (tex.addressing == gladdress) {
    return;
  }

  bind_texture(tex.id, tex.type, _active_tex);
  GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_WRAP_S, gladdress)); // U
  if (tex.type != GL_TEXTURE_1D || tex.type != GL_TEXTURE_1D_ARRAY) {
    GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_WRAP_T, gladdress)); // V
    if (tex.type == GL_TEXTURE_3D || tex.type == GL_TEXTURE_CUBE_MAP) {
      GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_WRAP_R, gladdress)); // W (?)
    }
  }
  tex.addressing = gladdress;
}

void gl_state::gen_texture_mipmaps(const texture_t& tex) {
  bind_texture(tex.id, tex.type, _active_tex);
  GL_CALL(glGenerateMipmap(tex.type));
}

GLenum gl_state::fbo_attachment_cast(r_test_buffer_flag flags) noexcept {
  const bool depth = +(flags & r_test_buffer_flag::depth);
  const bool stencil = +(flags & r_test_buffer_flag::stencil);
  if (depth && stencil) {
    return GL_DEPTH_STENCIL_ATTACHMENT;
  }
  if (depth) {
    return GL_DEPTH_ATTACHMENT;
  }
  if (stencil) {
    return GL_STENCIL_ATTACHMENT;
  }

  return 0;
}

auto gl_state::create_framebuffer(uint32 w, uint32 h, r_test_buffer_flag buffers,
                                  r_texture_format format) -> framebuffer_t {
  NTF_ASSERT(w > 0 && h > 0);
  const GLenum sd_attachment = fbo_attachment_cast(buffers);
  const GLenum fbbind = GL_DRAW_FRAMEBUFFER;

  GLuint id;
  GL_CALL(glGenFramebuffers(1, &id));
  GL_CALL(glBindFramebuffer(fbbind, id));
  _bound_fbos[FBO_BIND_WRITE] = id;

  GLuint rbos[2] = {0, 0};
  if (sd_attachment) {
    GL_CALL(glGenRenderbuffers(2, rbos));
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, rbos[0]));
    GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h));
    GL_CALL(glFramebufferRenderbuffer(fbbind, sd_attachment, GL_RENDERBUFFER, rbos[0]));
  } else {
    GL_CALL(glGenRenderbuffers(1, &rbos[1]));
  }
  GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, rbos[1]));
  GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, texture_format_cast(format), w, h));
  GL_CALL(glFramebufferRenderbuffer(fbbind, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbos[1]));

  GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, NULL_BINDING));

  NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);

  framebuffer_t fbo;
  fbo.id = id;
  fbo.sd_rbo = rbos[0];
  fbo.color_rbo = rbos[1];
  fbo.extent.x = w;
  fbo.extent.y = h;
  return fbo;
}

auto gl_state::create_framebuffer(uint32 w, uint32 h, r_test_buffer_flag buffers,
                                  const fbo_attachment_t* attachments,
                                  uint32 att_count) -> framebuffer_t {
  NTF_ASSERT(att_count <= MAX_FBO_ATTACHMENTS);
  NTF_ASSERT(attachments);
  NTF_ASSERT(w > 0 && h > 0);
  const GLenum sd_attachment = fbo_attachment_cast(buffers);
  const GLenum fbbind = GL_DRAW_FRAMEBUFFER;

  GLuint id;
  GL_CALL(glGenFramebuffers(1, &id));
  GL_CALL(glBindFramebuffer(fbbind, id));
  _bound_fbos[FBO_BIND_WRITE] = id;

  GLuint rbo{0};
  if (sd_attachment) {
    GL_CALL(glGenRenderbuffers(1, &rbo));
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
    GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h));
    GL_CALL(glFramebufferRenderbuffer(fbbind, sd_attachment, GL_RENDERBUFFER, rbo));
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, NULL_BINDING));
  }

  for (uint32 i = 0; i < att_count; ++i) {
    NTF_ASSERT(attachments[i].tex);
    const auto& tex = *attachments[i].tex;
    bind_texture(tex.id, tex.type, _active_tex);
    switch (tex.type) {
      case GL_TEXTURE_1D: {
        NTF_ASSERT(w == tex.extent.x && h == 1);
        GL_CALL(glFramebufferTexture1D(fbbind, GL_COLOR_ATTACHMENT0+i,
                                       tex.type, tex.id, attachments[i].level));
        break;
      }
      case GL_TEXTURE_2D: {
        NTF_ASSERT(w == tex.extent.x && h == tex.extent.y);
        GL_CALL(glFramebufferTexture2D(fbbind, GL_COLOR_ATTACHMENT0+i,
                                       tex.type, tex.id, attachments[i].level));
        break;
      }
      default: {
        NTF_UNREACHABLE();
        break;
      }
    }
  }

  NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);

  framebuffer_t fbo;
  fbo.id = id;
  fbo.sd_rbo = rbo;
  fbo.color_rbo = NULL_BINDING;
  fbo.extent.x = w;
  fbo.extent.y = h;
  return fbo;
}

void gl_state::destroy_framebuffer(const framebuffer_t& fbo) noexcept {
  NTF_ASSERT(fbo.id);
  if (_bound_fbos[FBO_BIND_WRITE] == fbo.id) {
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, DEFAULT_FBO));
    _bound_fbos[FBO_BIND_WRITE] = DEFAULT_FBO;
  } else if (_bound_fbos[FBO_BIND_READ] == fbo.id) {
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, DEFAULT_FBO));
    _bound_fbos[FBO_BIND_READ] = DEFAULT_FBO;
  }

  GLuint id = fbo.id;
  GL_CALL(glDeleteFramebuffers(1, &id));
  if (fbo.sd_rbo) {
    GLuint rbo = fbo.sd_rbo;
    GL_CALL(glDeleteRenderbuffers(1, &rbo));
  }
  if (fbo.color_rbo) {
    GLuint rbo = fbo.color_rbo;
    GL_CALL(glDeleteRenderbuffers(1, &rbo));
  }
}

void gl_state::bind_framebuffer(GLuint id, fbo_binding binding) noexcept {
  GLenum fb;
  switch (binding) {
    case FBO_BIND_WRITE: {
      if (_bound_fbos[FBO_BIND_WRITE] == id) {
        return;
      }
      fb = GL_DRAW_FRAMEBUFFER;
      _bound_fbos[FBO_BIND_WRITE] = id;
      break;
    }
    case FBO_BIND_READ: {
      if (_bound_fbos[FBO_BIND_READ] == id) {
        return;
      }
      _bound_fbos[FBO_BIND_READ] = id;
      fb = GL_READ_FRAMEBUFFER;
      break;
    }
    case FBO_BIND_BOTH: {
      if (_bound_fbos[FBO_BIND_READ] == id && _bound_fbos[FBO_BIND_WRITE] == id) {
        return;
      }
      _bound_fbos[FBO_BIND_READ] = id;
      _bound_fbos[FBO_BIND_WRITE] = id;
      fb = GL_FRAMEBUFFER;
      break;
    }
    default: {
      NTF_UNREACHABLE();
    }
  }
  GL_CALL(glBindFramebuffer(fb, id));
}

void gl_state::bind_attributes(const r_attrib_descriptor* attrs, uint32 count, 
                               size_t stride) noexcept {
  NTF_ASSERT(attrs);
  NTF_ASSERT(count > 0);
  NTF_ASSERT(stride > 0);
  for (uint32 i = 0; i < count; ++i) {
    const auto& attr = attrs[i];
    // TODO: Don't re-enable already enabled attribs (and disable others)
    GL_CALL(glEnableVertexAttribArray(attr.location));

    const uint32 type_dim = r_attrib_type_dim(attr.type);
    NTF_ASSERT(type_dim);
    const GLenum gl_underlying_type = gl_state::attrib_underlying_type_cast(attr.type);
    NTF_ASSERT(gl_underlying_type);
    GL_CALL(glVertexAttribPointer(
      attr.location,
      type_dim,
      gl_underlying_type,
      GL_FALSE, // Don't normalize
      stride,
      reinterpret_cast<void*>(attr.offset)
    ));
  }
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

void gl_state::prepare_draw_target(GLuint fb, r_clear_flag flags,
                                   const uvec4& vp, const color4& col) noexcept {
  bind_framebuffer(fb, gl_state::FBO_BIND_WRITE);
  GL_CALL(glViewport(vp.x, vp.y, vp.z, vp.w));
  if (!+(flags & r_clear_flag::color)) {
    return;
  }
  GL_CALL(glClearColor(col.r, col.g, col.b, col.a));
  GL_CALL(glClear(clear_bit_cast(flags)));
}

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

void gl_context::debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                 GLsizei, const GLchar* msg, const void*) {
  // auto* ctx = reinterpret_cast<gl_context*>(const_cast<void*>(user_ptr));

  std::string_view severity_msg = [severity]() {
    switch (severity) {
      case GL_DEBUG_SEVERITY_HIGH:          return "HIGH";
      case GL_DEBUG_SEVERITY_MEDIUM:        return "MEDIUM";
      case GL_DEBUG_SEVERITY_LOW:           return "LOW";
      case GL_DEBUG_SEVERITY_NOTIFICATION:  return "NOTIFICATION";

      default: break;
    }
    return "UNKNOWN_SEVERITY";
  }();

  std::string_view type_msg = [type]() {
    switch (type) {
      case GL_DEBUG_TYPE_ERROR:               return "ERROR";
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
      case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITY";
      case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
      case GL_DEBUG_TYPE_MARKER:              return "MARKER";
      case GL_DEBUG_TYPE_PUSH_GROUP:          return "PUSH_GROUP";
      case GL_DEBUG_TYPE_POP_GROUP:           return "POP_GROUP";
      case GL_DEBUG_TYPE_OTHER:               return "OTHER";

      default: break;
    };
    return "UNKNOWN_TYPE";
  }();

  std::string_view src_msg = [src]() {
    switch (src) {
      case GL_DEBUG_SOURCE_API:             return "API";
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW_SYSTEM";
      case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER_COMPILER";
      case GL_DEBUG_SOURCE_THIRD_PARTY:     return "THIRD_PARTY";
      case GL_DEBUG_SOURCE_APPLICATION:     return "APPLICATION";
      case GL_DEBUG_SOURCE_OTHER:           return "OTHER";

      default: break;
    };
    return "UNKNOWN_SOURCE";
  }();

  if (type == GL_DEBUG_TYPE_ERROR) {
    SHOGLE_LOG(error, "[ntf::gl_context][GL_DEBUG][{}][{}][{}][{}]\n\t{}",
               severity_msg, type_msg, src_msg, id, msg);
  } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    SHOGLE_LOG(verbose, "[ntf::gl_context][GL_DEBUG][{}][{}][{}][{}]\n\t{}",
               severity_msg, type_msg, src_msg, id, msg);
  } else {
    SHOGLE_LOG(debug, "[ntf::gl_context][GL_DEBUG][{}][{}][{}][{}]\n\t{}",
               severity_msg, type_msg, src_msg, id, msg);
  }
}

gl_context::gl_context() :
  _state{*this} {
  _state.init(gl_state::init_data_t{
    .dbg = gl_context::debug_callback,
  });
  _vao = _state.create_vao();
  _state.bind_vao(_vao.id);
}

gl_context::~gl_context() noexcept {}

r_context::ctx_meta_t gl_context::query_meta() const {
  r_context::ctx_meta_t meta;
  meta.api = r_api::opengl;
  GL_CALL(meta.name_str = reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
  GL_CALL(meta.vendor_str = reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
  GL_CALL(meta.version_str = reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  meta.tex_max_layers = _state.tex_limits().max_layers;
  meta.tex_max_extent = _state.tex_limits().max_dim;
  meta.tex_max_extent3d = _state.tex_limits().max_dim3d;
  return meta;
}

r_buffer_handle gl_context::create_buffer(const r_buffer_descriptor& desc) {
  gl_state::buffer_t buffer = _state.create_buffer(
    desc.type, desc.flags, desc.size, desc.data
  );
  NTF_ASSERT(buffer.id);
  auto handle = _buffers.acquire();
  _buffers.get(handle) = buffer;
  return handle;
}

void gl_context::update_buffer(r_buffer_handle buf, const r_buffer_data& desc) {
  auto& buffer = _buffers.get(buf);
  _state.update_buffer(buffer, desc.data, desc.size, desc.offset);
}

void gl_context::destroy_buffer(r_buffer_handle buf) noexcept {
  auto& buffer = _buffers.get(buf);
  _state.destroy_buffer(buffer);
  _buffers.push(buf);
}

r_texture_handle gl_context::create_texture(const r_texture_descriptor& desc) {
  gl_state::texture_t texture = _state.create_texture(
    desc.type, desc.format, desc.sampler, desc.addressing,
    desc.extent, desc.layers, desc.levels
  );
  NTF_ASSERT(texture.id);
  if (desc.images) {
    for (const auto& img : desc.images) {
      _state.update_texture_data(
        texture, img.texels, img.format, img.offset, img.layer, img.layer
      );
    }
    if (desc.gen_mipmaps && texture.levels > 1) {
      _state.gen_texture_mipmaps(texture);
    }
  }
  auto handle = _textures.acquire();
  _textures.get(handle) = texture;
  return handle;
}

void gl_context::update_texture(r_texture_handle tex, const r_texture_data& desc) {
  auto& texture = _textures.get(tex);
  if (desc.images) {
    for (const auto& img : desc.images) {
      _state.update_texture_data(
        texture, img.texels, img.format, img.offset, img.layer, img.layer
      );
    }
  }
  if (desc.addressing) {
    _state.update_texture_addressing(texture, desc.addressing.value());
  }
  if (desc.sampler) {
    _state.update_texture_sampler(texture, desc.sampler.value());
  }
}

void gl_context::destroy_texture(r_texture_handle tex) noexcept {
  auto& texture = _textures.get(tex);
  _state.destroy_texture(texture);
  _textures.push(tex);
}

r_framebuffer_handle gl_context::create_framebuffer(const r_framebuffer_descriptor& desc) {
  r_framebuffer_handle handle;
  if (desc.attachments) {
    NTF_ASSERT(desc.attachments);
    std::vector<gl_state::fbo_attachment_t> atts;
    atts.reserve(desc.attachments.size());
    for (const auto& att : desc.attachments) {
      atts.emplace_back(&_textures.get(att.handle), att.layer, att.level);
    }

    gl_state::framebuffer_t framebuffer = _state.create_framebuffer(
      desc.extent.x, desc.extent.y, desc.test_buffers, atts.data(), atts.size()
    );
    NTF_ASSERT(framebuffer.id);
    handle = _framebuffers.acquire();
    _framebuffers.get(handle) = framebuffer;
  } else {
    NTF_ASSERT(desc.color_buffer_format);
    gl_state::framebuffer_t framebuffer = _state.create_framebuffer(
      desc.extent.x, desc.extent.y, desc.test_buffers, *desc.color_buffer_format
    );
    NTF_ASSERT(framebuffer.id);
    handle = _framebuffers.acquire();
    _framebuffers.get(handle) = framebuffer;
  }

  return handle;
}

void gl_context::destroy_framebuffer(r_framebuffer_handle fb) noexcept {
  auto& framebuffer = _framebuffers.get(fb);
  _state.destroy_framebuffer(framebuffer);
  _framebuffers.push(fb);
}

r_shader_handle gl_context::create_shader(const r_shader_descriptor& desc) {
  NTF_ASSERT(desc.source);
  // TODO: Concatenate sources
  gl_state::shader_t shader = _state.create_shader(desc.type, desc.source[0]);
  auto handle = _shaders.acquire();
  _shaders.get(handle) = shader;
  return handle;
}

void gl_context::destroy_shader(r_shader_handle shad) noexcept {
  auto& shader = _shaders.get(shad);
  _state.destroy_shader(shader);
  _shaders.push(shad);
}

r_pipeline_handle gl_context::create_pipeline(const r_pipeline_descriptor& desc,
                                              weak_ref<r_context::vertex_attrib_t> attrib,
                                              r_context::uniform_map& uniforms) {
  NTF_ASSERT(desc.stages);
  std::vector<gl_state::shader_t*> shads;
  shads.reserve(desc.stages.size());
  for (const auto& stage : desc.stages) {
    shads.emplace_back(&_shaders.get(stage));
  }
  gl_state::program_t prog = _state.create_program(
    shads.data(), shads.size(), desc.primitive
  );

  _state.query_program_uniforms(prog, uniforms);
  prog.layout = attrib;

  NTF_ASSERT(prog.id);
  auto handle = _programs.acquire();
  _programs.get(handle) = prog;
  return handle;
}

void gl_context::destroy_pipeline(r_pipeline_handle pipeline) noexcept {
  auto& prog = _programs.get(pipeline);
  _state.destroy_program(prog);
  _programs.push(pipeline);
}

void gl_context::submit(win_handle_t win, const r_context::command_map& cmds) {
  r_window::gl_set_current(win);
  _state.bind_vao(_vao.id);
  for (const auto& [fb, list] : cmds) {
    _state.prepare_draw_target(fb ? _framebuffers.get(fb).id : gl_state::DEFAULT_FBO,
                               list.clear, list.viewport, list.color);

    for (const auto& cmd_ref : list.cmds) {
      const auto& cmd = *cmd_ref;

      NTF_ASSERT(cmd.count);
      NTF_ASSERT(cmd.vertex_buffer);
      NTF_ASSERT(cmd.pipeline);

      const auto& vbo = _buffers.get(cmd.vertex_buffer);
      const auto& prog = _programs.get(cmd.pipeline);
      NTF_ASSERT(vbo.id);
      NTF_ASSERT(vbo.type == GL_ARRAY_BUFFER);
      NTF_ASSERT(prog.id);

      bool rebind = _state.bind_buffer(vbo.id, vbo.type);
      rebind = (_state.bind_program(prog.id) || rebind);
      NTF_ASSERT(prog.primitive);
      if (rebind) {
        auto& layout = prog.layout;
        _state.bind_attributes(
          layout->descriptors.data(), layout->descriptors.size(), layout->stride
        );
      }

      if (cmd.index_buffer) {
        auto& ebo = _buffers.get(cmd.index_buffer);
        NTF_ASSERT(ebo.type == GL_ELEMENT_ARRAY_BUFFER);
        _state.bind_buffer(ebo.id, ebo.type);
      }

      for (const auto& bind : cmd.textures) {
        const auto& tex = _textures.get(bind->handle);
        _state.bind_texture(tex.id, tex.type, bind->index);
      }

      for (const auto& unif : cmd.uniforms) {
        _state.push_uniform(static_cast<uint32>(unif->location), unif->type, unif->data);
      }

      if (cmd.index_buffer) {
        const void* offset = reinterpret_cast<const void*>(cmd.offset*sizeof(uint32));
        const GLenum format = GL_UNSIGNED_INT;
        if (cmd.instances) {
          GL_CALL(glDrawElementsInstanced(
            prog.primitive, cmd.count, format, offset, cmd.instances
          ));
        } else {
          GL_CALL(glDrawElements(
            prog.primitive, cmd.count, format, offset
          ));
        }
      } else {
        if (cmd.instances) {
          GL_CALL(glDrawArraysInstanced(
            prog.primitive, cmd.offset, cmd.count, cmd.instances
          ));
        } else {
          GL_CALL(glDrawArrays(
            prog.primitive, cmd.offset, cmd.count
          ));
        }
      }
    }
  }
}

void gl_context::swap_buffers(win_handle_t win) {
  r_window::gl_swap_buffers(win);
}

// void gl_context::draw_text(const font& font, vec2 pos, float scale, std::string_view text) const {
//   NTF_ASSERT(valid());
//
//   if (!font.valid() || font.empty()) {
//     SHOGLE_LOG(warning, "[ntf::gl_context::draw_text] Attempted to draw \"{}\" with empty font", 
//                text);
//     return;
//   }
//
//   glBindVertexArray(font.vao());
//
//   float x = pos.x, y = pos.y;
//   const auto& atlas = font.atlas();
//   for (const auto c : text) {
//     GLuint tex;
//     font_glyph glyph;
//     if (atlas.find(c) != atlas.end()) {
//       std::tie(tex, glyph) = atlas.at(c);
//     } else {
//       std::tie(tex, glyph) = atlas.at('?');
//     }
//
//     float xpos = x + glyph.bearing.x*scale;
//     float ypos = y - glyph.bearing.y*scale;
//
//     float w = glyph.size.x*scale;
//     float h = glyph.size.y*scale;
//
//     float vert[6][4] {
//       { xpos,     ypos + h, 0.0f, 1.0f },
//       { xpos,     ypos,     0.0f, 0.0f },
//       { xpos + w, ypos,     1.0f, 0.0f },
//
//       { xpos,     ypos + h, 0.0f, 1.0f },
//       { xpos + w, ypos,     1.0f, 0.0f },
//       { xpos + w, ypos + h, 1.0f, 1.0f }
//     };
//     glBindTexture(GL_TEXTURE_2D, tex);
//
//     glBindBuffer(GL_ARRAY_BUFFER, font.vbo());
//     glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vert), vert);
//
//     glDrawArrays(GL_TRIANGLES, 0, 6);
//
//     x += (glyph.advance >> 6)*scale;
//   }
//   glBindVertexArray(0);
//   glBindTexture(GL_TEXTURE_2D, 0);
// }
} // namespace ntf
