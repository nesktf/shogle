#include "./state.hpp"

namespace ntf {

static constexpr size_t FBO_INDEX_READ = 0;
static constexpr size_t FBO_INDEX_WRITE = 1;
static constexpr bool   NO_BINDING_CHANGE = false;
static constexpr uint32 MAX_TEXTURE_LEVEL = 7;

auto gl_state::viewport_data::coords(uint32 x, uint32 y, uint32 w, uint32 h) noexcept
                                                                      -> gl_state::viewport_data& {
  _coords.x = x;
  _coords.y = y;
  _coords.z = w;
  _coords.w = h;
  return *this;
}

auto gl_state::viewport_data::coords(uvec2 pos, uvec2 size) noexcept -> gl_state::viewport_data& {
  return coords(pos.x, pos.y, size.x, size.y);
}

auto gl_state::viewport_data::size(uint32 w, uint32 h) noexcept -> gl_state::viewport_data& {
  return coords(_coords.x, _coords.y, w, h);
}

auto gl_state::viewport_data::size(uvec2 size) noexcept-> gl_state::viewport_data&  {
  return this->size(size.x, size.y);
}

auto gl_state::viewport_data::pos(uint32 x, uint32 y) noexcept -> gl_state::viewport_data& {
  return coords(x, y, _coords.z, _coords.w);
}

auto gl_state::viewport_data::pos(uvec2 pos) noexcept -> gl_state::viewport_data& {
  return this->pos(pos.x, pos.y);
}

auto gl_state::viewport_data::color(float32 r, float32 g, float32 b, float32 a) noexcept
                                                                      -> gl_state::viewport_data& {
  _color.r = r;
  _color.g = g;
  _color.b = b;
  _color.a = a;
  return *this;
}

auto gl_state::viewport_data::color(float32 r, float32 g, float32 b) noexcept
                                                                      -> gl_state::viewport_data& {
  return color(r, g, b, _color.a);
}

auto gl_state::viewport_data::color(color4 color) noexcept -> gl_state::viewport_data& {
  return this->color(color.r, color.g, color.b, color.a);
}

auto gl_state::viewport_data::color(color3 color) noexcept -> gl_state::viewport_data& {
  return this->color(color.r, color.g, color.b);
}

auto gl_state::viewport_data::flags(r_clear_flag flags) noexcept -> gl_state::viewport_data& {
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
  _flags = clear_bits;
  return *this;
}

r_clear_flag gl_state::viewport_data::flags() const noexcept {
  r_clear_flag clear_bits{r_clear_flag::none};
  if (_flags & GL_COLOR_BUFFER_BIT) {
    clear_bits |= r_clear_flag::color;
  }
  if (_flags & GL_DEPTH_BUFFER_BIT) {
    clear_bits |= r_clear_flag::depth;
  }
  if (_flags & GL_STENCIL_BUFFER_BIT) {
    clear_bits |= r_clear_flag::stencil;
  }
  return clear_bits;
}

gl_state::gl_state() noexcept {
  std::memset(_buff_active, 0, sizeof(GLuint)*r_buffer_type_count);
  std::memset(_fbo, 0, sizeof(GLuint)*fbo_bind_count);
}

void gl_state::init() noexcept {
  GLint max_tex;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tex);
  _tex_active.resize(max_tex, std::make_pair(0, 0));

  GLint max_tex_lay;
  glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_tex_lay);
  _tex_max_layers = max_tex_lay;

  GLint max_tex_dim;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_dim);
  _tex_max_dim = max_tex_dim;

  GLint max_tex_dim3d;
  glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max_tex_dim3d);
  _tex_max_dim3d = max_tex_dim3d;

  GLint max_fbo_attach;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_fbo_attach);
  _fbo_max_attachments = max_fbo_attach;
}

bool gl_state::bind_buffer(r_buffer_type type, GLuint id) noexcept {
  auto& buff = _buff_active[static_cast<size_t>(type)];
  if (buff == id) {
    return NO_BINDING_CHANGE;
  }

  const GLenum gltype = buffer_type_cast(type);
  glBindBuffer(gltype, id);
  buff = id;

  return true;
}

GLuint gl_state::buffer(r_buffer_type type) const noexcept {
  return _buff_active[static_cast<size_t>(type)];
}

GLuint gl_state::make_buffer(r_buffer_type type, size_t size) noexcept {
  GLuint id;
  glGenBuffers(1, &id);
  bind_buffer(type, id);
  const GLenum gltype = buffer_type_cast(type);
  const GLbitfield glflags =
    GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT; // TODO: Custom flags?
  glBufferStorage(gltype, size, nullptr, glflags);
  return id;
}

void gl_state::destroy_buffer(r_buffer_type type, GLuint id) noexcept {
  auto& buff = _buff_active[static_cast<size_t>(type)];
  if (buff == id) {
    bind_buffer(type, 0);
  }
  glDeleteBuffers(1, &id);
}

void gl_state::buffer_data(GLuint id, r_buffer_type type,
                           const void* data, size_t size, size_t offset) noexcept {
  bind_buffer(type, id);
  const GLenum gltype = buffer_type_cast(type);
#ifndef NDEBUG
  GLint sz;
  glGetBufferParameteriv(gltype, GL_BUFFER_SIZE, &sz);
  NTF_ASSERT(size+offset <= static_cast<size_t>(sz));
#endif
  glBufferSubData(gltype, offset, size, data);
}

bool gl_state::bind_vao(GLuint id) noexcept {
  if (_vao == id) {
    return NO_BINDING_CHANGE;
  }

  glBindVertexArray(id);
  _vao = id;
  return true;
}


GLuint gl_state::vao() const noexcept {
  return _vao;
}

GLuint gl_state::make_vao() noexcept {
  GLuint id;
  glGenVertexArrays(1, &id);
  bind_vao(id);
  return id;
}

void gl_state::destroy_vao(GLuint id) noexcept {
  if (_vao == id) {
    bind_vao(0);
  }
  glDeleteVertexArrays(1, &id);
}

bool gl_state::bind_framebuffer(GLuint id, fbo_bind_flag binding) noexcept {
  const GLenum gltype = fbo_bind_cast(binding);

  if (gltype == GL_READ_FRAMEBUFFER && _fbo[FBO_INDEX_READ] == id) {
    return NO_BINDING_CHANGE;
  }

  if (gltype == GL_DRAW_FRAMEBUFFER && _fbo[FBO_INDEX_WRITE] == id) {
    return NO_BINDING_CHANGE;
  }

  if (gltype == GL_FRAMEBUFFER && _fbo[FBO_INDEX_READ] == id && _fbo[FBO_INDEX_WRITE] == id) {
    return NO_BINDING_CHANGE;
  }

  glBindBuffer(gltype, id);
  if (binding & FBO_BIND_READ) {
    _fbo[FBO_INDEX_READ] = id;
  }
  if (binding & FBO_BIND_WRITE) {
    _fbo[FBO_INDEX_WRITE] = id;
  }

  return true;
}

GLuint gl_state::framebuffer(fbo_bind_flag binding) const noexcept {
  if (binding & FBO_BIND_READ) {
    return _fbo[FBO_INDEX_READ];
  } else if (binding & FBO_BIND_WRITE) {
    return _fbo[FBO_INDEX_WRITE];
  }

  NTF_UNREACHABLE();
  return 0;
}

GLuint gl_state::make_framebuffer() noexcept {
  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  return fbo;
}

void gl_state::destroy_framebuffer(GLuint id) noexcept {
  if (_fbo[FBO_INDEX_READ] == id) {
    _fbo[FBO_INDEX_READ] = 0;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }
  if (_fbo[FBO_INDEX_WRITE] == id) {
    _fbo[FBO_INDEX_WRITE] = 0;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }
  glDeleteFramebuffers(1, &id);
}

void gl_state::framebuffer_tex_attachment(GLuint fbo, GLuint texture, r_texture_type tex_type,
                                          uint32 index, uint32 level) noexcept {
  NTF_ASSERT(level > 0 && level <= MAX_TEXTURE_LEVEL);
  NTF_ASSERT(index < _fbo_max_attachments);

  bind_framebuffer(fbo, FBO_BIND_WRITE);
  const GLenum fbtype = fbo_bind_cast(FBO_BIND_WRITE);
  bind_texture(texture, _tex_active_index, tex_type);
  const GLenum textype = texture_type_cast(tex_type);

  switch (textype) {
    case GL_TEXTURE_1D: {
      glFramebufferTexture1D(fbtype, GL_COLOR_ATTACHMENT0+index, textype, texture, level);
      break;
    }

    case GL_TEXTURE_2D: {
      glFramebufferTexture2D(fbtype, GL_COLOR_ATTACHMENT0+index, textype, texture, level);
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };
}

void gl_state::framebuffer_rbo_attachment(GLuint fbo, GLuint rbo, rbo_attachment attach) noexcept {
  bind_framebuffer(fbo, FBO_BIND_WRITE);
  const GLenum fbtype = fbo_bind_cast(FBO_BIND_WRITE);
  bind_renderbuffer(rbo);

  glFramebufferRenderbuffer(fbtype, static_cast<GLenum>(attach), GL_RENDERBUFFER, rbo);
}

auto gl_state::check_framebuffer(GLuint id) noexcept -> fbo_status {
  bind_framebuffer(id, FBO_BIND_WRITE);
  return static_cast<fbo_status>(glCheckFramebufferStatus(fbo_bind_cast(FBO_BIND_WRITE)));
}

bool gl_state::bind_renderbuffer(GLuint id) noexcept {
  if (_rbo == id) {
    return NO_BINDING_CHANGE;
  }

  glBindRenderbuffer(GL_RENDERBUFFER, id);
  _rbo = id;
  return true;
}

GLuint gl_state::renderbuffer() const noexcept {
  return _rbo;
}

GLuint gl_state::make_renderbuffer(uint32 w, uint32 h) noexcept {
  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  bind_renderbuffer(rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  return rbo;
}

void gl_state::destroy_renderbuffer(GLuint id) noexcept {
  if (_rbo == id) {
    bind_renderbuffer(0);
  }
  glDeleteRenderbuffers(1, &id);
}

bool gl_state::bind_program(GLuint id) noexcept {
  if (_shad_prog == id) {
    return NO_BINDING_CHANGE;
  }

  glUseProgram(id);
  _shad_prog = id;
  return true;
}

GLuint gl_state::program() const noexcept {
  return _shad_prog;
}

GLuint gl_state::make_program() const noexcept {
  return glCreateProgram();
}

void gl_state::destroy_program(GLuint id) noexcept {
  if (_shad_prog == id) {
    bind_program(0);
  }
  glDeleteProgram(id);
}

optional<std::string_view> gl_state::link_program(GLuint program, const GLuint* shaders,
                                                  uint32 count) noexcept {
  for (uint32 i = 0; i < count; ++i) {
    glAttachShader(program, shaders[i]);
  }
  glLinkProgram(program);

  int succ;
  glGetProgramiv(program, GL_LINK_STATUS, &succ);
  if (!succ) {
    char msg[1024];
    glGetShaderInfoLog(shaders[0], 1024, nullptr, msg);
    _shader_err = std::string{msg};
    return {_shader_err};
  }

  return nullopt;
}

GLuint gl_state::make_shader(r_shader_type type) noexcept {
  return glCreateShader(shader_type_cast(type));
}

void gl_state::destroy_shader(GLuint id) noexcept {
  glDeleteShader(id);
}

optional<std::string_view> gl_state::compile_shader(GLuint shader, std::string_view src) noexcept {
  const char* src_data = src.data();

  glShaderSource(shader, 1, &src_data, nullptr);
  glCompileShader(shader);

  int succ;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    char msg[1024];
    glGetShaderInfoLog(shader, 1024, nullptr, msg);
    _shader_err = std::string{msg};
    return {_shader_err};
  }

  return nullopt;
}

GLenum gl_state::bind_texture(GLuint id, uint32 index, r_texture_type type) noexcept {
  NTF_ASSERT(index < _tex_active.size());
  auto& [tex_id, tex_type] = _tex_active[index];
  if (tex_id == id) {
    return NO_BINDING_CHANGE;
  }

  const GLenum gltype = texture_type_cast(type);
  if (index != _tex_active_index) {
    glActiveTexture(GL_TEXTURE0+index);
  }
  glBindTexture(gltype, id);

  tex_id = id;
  tex_type = gltype;
  _tex_active_index = static_cast<size_t>(index);
  return gltype;
}

GLuint gl_state::make_texture(r_texture_type type, r_texture_format format,
                              uint32 level, uint32 count, uvec3 dim) noexcept {
  NTF_ASSERT(count < _tex_max_layers);
  NTF_ASSERT(level > 0 && level <= MAX_TEXTURE_LEVEL);

  const GLenum glformat = texture_format_cast(format);

  GLuint id;
  glGenTextures(1, &id);
  bind_texture(id, _tex_active_index, type);
  const GLenum gltype = texture_type_cast(type);

  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(dim.x > 0);
      NTF_ASSERT(count > 0);
      glTexStorage2D(gltype, level, glformat, dim.x, count);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(dim.x > 0 && dim.x <= _tex_max_dim);
      glTexStorage1D(gltype, level, glformat, dim.x);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(dim.x > 0 && dim.y > 0 && dim.x <= _tex_max_dim && dim.y <= _tex_max_dim);
      NTF_ASSERT(count > 0);
      glTexStorage3D(gltype, level, glformat, dim.x, dim.y, count);
      break;
    }

    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(dim.x == dim.y);
      [[fallthrough]];
    }
    case GL_TEXTURE_2D: {
      NTF_ASSERT(dim.x > 0 && dim.y > 0 && dim.x <= _tex_max_dim && dim.y <= _tex_max_dim);
      glTexStorage2D(gltype, level, glformat, dim.x, dim.y);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(dim.x > 0 && dim.y > 0 && dim.z > 0);
      NTF_ASSERT(dim.x <= _tex_max_dim3d && dim.y <= _tex_max_dim3d && dim.y <= _tex_max_dim3d);
      glTexStorage3D(gltype, level, glformat, dim.x, dim.y, dim.z);
      break;
    };

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };

  return id;
}

void gl_state::destroy_texture(GLuint id) noexcept {
  for (auto& [tex_id, tex_type] : _tex_active) {
    if (tex_id == id) {
      tex_id = 0;
      tex_type = 0;
    }
  }
  glDeleteTextures(1, &id);
}

void gl_state::texture_data(GLuint id, r_texture_type type, r_texture_format format,
                            const uint8* texels, uint32 index, uint32 count, uint32 mipmap,
                            uvec3 dim, uvec3 offset, bool gen_mipmaps) noexcept {
  bind_texture(id, _tex_active_index, type);
  const GLenum gltype = texture_type_cast(type);
  const GLenum glformat = texture_format_cast(format);
  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(offset.x < dim.x);
      NTF_ASSERT(index < count);
      glTexSubImage2D(gltype, mipmap, offset.x, index, dim.x, count, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(offset.x < dim.x);
      glTexSubImage1D(gltype, mipmap, offset.x, dim.x, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(offset.x < dim.x && offset.y < dim.y);
      NTF_ASSERT(index < count);
      glTexSubImage3D(gltype, mipmap, offset.x, offset.y, index, dim.x, dim.y, count, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_2D: {
      NTF_ASSERT(offset.x < dim.x && offset.y < dim.y);
      glTexSubImage2D(gltype, mipmap, offset.x, offset.y, dim.x, dim.y, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }
    
    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(offset.x < dim.x);
      const GLenum glface = GL_TEXTURE_CUBE_MAP_POSITIVE_X+index; 
      glTexSubImage2D(glface, mipmap, offset.x, offset.y, dim.x, dim.y, glformat,
                      GL_UNSIGNED_BYTE, texels);
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(offset.x < dim.x && offset.y < dim.y && offset.z < dim.z);
      glTexSubImage3D(gltype, mipmap, offset.x, offset.y, offset.z, dim.x, dim.y, dim.z,
                      glformat, GL_UNSIGNED_BYTE, texels);
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }

  if (gen_mipmaps) {
    glGenerateMipmap(gltype);
  }
}

void gl_state::texture_sampler(GLuint id, r_texture_type type,
                               r_texture_sampler sampler, uint32 level) noexcept {
  NTF_ASSERT(level > 0 && level <= MAX_TEXTURE_LEVEL);
  bind_texture(id, _tex_active_index, type);
  const GLenum gltype = texture_type_cast(type);
  const GLenum glsamplermin = texture_sampler_cast(sampler, (level > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  glTexParameteri(gltype, GL_TEXTURE_MAG_FILTER, glsamplermag);
  glTexParameteri(gltype, GL_TEXTURE_MIN_FILTER, glsamplermin);
}

void gl_state::texture_addressing(GLuint id, r_texture_type type,
                                  r_texture_address addressing) noexcept {
  bind_texture(id, _tex_active_index, type);
  const GLenum gltype = texture_type_cast(type);
  const GLenum gladdress = texture_addressing_cast(addressing);
  glTexParameteri(gltype, GL_TEXTURE_WRAP_S, gladdress); // U
  if (gltype != GL_TEXTURE_1D || gltype != GL_TEXTURE_1D_ARRAY) {
    glTexParameteri(gltype, GL_TEXTURE_WRAP_T, gladdress); // V
    if (gltype == GL_TEXTURE_3D || gltype == GL_TEXTURE_CUBE_MAP) {
      glTexParameteri(gltype, GL_TEXTURE_WRAP_R, gladdress); // W (?)
    }
  }
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

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
  return 0;
}

GLenum gl_state::primitive_cast(r_primitive primitive) noexcept {
  switch (primitive) {
    case r_primitive::points:         return GL_POINTS;
    case r_primitive::triangles:      return GL_TRIANGLES;
    case r_primitive::triangle_fan:   return GL_TRIANGLE_FAN;
    case r_primitive::lines:          return GL_LINES;
    case r_primitive::line_strip:     return GL_LINE_STRIP;
    case r_primitive::triangle_strip: return GL_TRIANGLE_STRIP;

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
  return 0;
}

GLenum gl_state::buffer_type_cast(r_buffer_type type) noexcept {
  switch(type) {
    case r_buffer_type::vertex:   return GL_ARRAY_BUFFER;
    case r_buffer_type::index:    return GL_ELEMENT_ARRAY_BUFFER;
    case r_buffer_type::uniform:  return GL_UNIFORM_BUFFER;

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };
  return 0;
}

GLenum gl_state::shader_type_cast(r_shader_type type) noexcept {
  switch (type) {
    case r_shader_type::vertex:               return GL_VERTEX_SHADER;
    case r_shader_type::fragment:             return GL_FRAGMENT_SHADER;
    case r_shader_type::geometry:             return GL_GEOMETRY_SHADER;
    case r_shader_type::tesselation_eval:     return GL_TESS_EVALUATION_SHADER;
    case r_shader_type::tesselation_control:  return GL_TESS_CONTROL_SHADER;

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
  return 0;
}

GLenum gl_state::fbo_bind_cast(fbo_bind_flag flags) noexcept {
  if ((flags & FBO_BIND_READ) && (flags & FBO_BIND_WRITE)) {
    return GL_FRAMEBUFFER;
  } else if (flags & FBO_BIND_READ) {
    return GL_READ_FRAMEBUFFER;
  } else if (flags & FBO_BIND_WRITE) {
    return GL_DRAW_FRAMEBUFFER;
  }

  NTF_UNREACHABLE();
  return 0;
}

GLenum gl_state::check_error(const char* file, int line) noexcept {
  GLenum err{};
  while ((err = glGetError()) != GL_NO_ERROR) {
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
    SHOGLE_LOG(error, "[ntf::gl_check_error] GL error ({}) | \"{}\":{} -> {}",
               err, file, line, err_str);
  }
  return err;
}

GLenum gl_state::texture_type_cast(r_texture_type type) noexcept {
  switch (type) {
    case r_texture_type::texture1d:       return GL_TEXTURE_1D;
    case r_texture_type::texture1d_array: return GL_TEXTURE_1D_ARRAY;
    case r_texture_type::texture2d:       return GL_TEXTURE_2D;
    case r_texture_type::texture2d_array: return GL_TEXTURE_2D_ARRAY;
    case r_texture_type::texture3d:       return GL_TEXTURE_3D;
    case r_texture_type::cubemap:         return GL_TEXTURE_CUBE_MAP;

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };
  return 0;
}

GLenum gl_state::texture_format_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::mono:    return GL_RED;
    case r_texture_format::rgb:     return GL_RGB;
    case r_texture_format::rgba:    return GL_RGBA;

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };
  return 0;
}

GLenum gl_state::texture_sampler_cast(r_texture_sampler sampler, bool mipmaps) noexcept {
  switch (sampler) {
    case r_texture_sampler::nearest:  return mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST;
    case r_texture_sampler::linear:   return mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    // TODO: change this?
    // case r_texture_sampler::linear: {
    //   if (mipmaps) {
    //     if (lin_lvl) {
    //       return lin_lvls ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
    //     } else {
    //       return lin_lvl ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
    //     }
    //   } else {
    //     return GL_LINEAR;
    //   }
    //   break;
    // }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }
  return 0;
}

GLenum gl_state::texture_addressing_cast(r_texture_address address) noexcept {
  switch (address) {
    case r_texture_address::clamp_border:         return GL_CLAMP_TO_BORDER;
    case r_texture_address::repeat:               return GL_REPEAT;
    case r_texture_address::repeat_mirrored:      return GL_MIRRORED_REPEAT;
    case r_texture_address::clamp_edge:           return GL_CLAMP_TO_EDGE;
    case r_texture_address::clamp_edge_mirrored:  return GL_MIRROR_CLAMP_TO_EDGE;

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };
  return 0;
}


bool gl_state::validate_descriptor(const r_attrib_descriptor&) const noexcept {
  return true;
}

bool gl_state::validate_descriptor(const r_texture_descriptor& desc) const noexcept {
  // TODO: Validate dimensions
  return 
    desc.count > 0 &&
    !(desc.count > 1 && desc.type == r_texture_type::texture3d) &&
    !(desc.count != 6 && desc.type == r_texture_type::cubemap);
}

bool gl_state::validate_descriptor(const r_buffer_descriptor&) const noexcept {
  return true;
}

bool gl_state::validate_descriptor(const r_pipeline_descriptor& desc) const noexcept {
  return !(!desc.stages || desc.stage_count < 2);
}

bool gl_state::validate_descriptor(const r_shader_descriptor&) const noexcept {
  return true;
}


bool gl_state::validate_descriptor(const r_framebuffer_descriptor&) const noexcept {
  return true;
}

} // namespace ntf
