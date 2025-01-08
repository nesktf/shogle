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

static GLenum gl_check_error(const char* file, int line) noexcept {
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
    SHOGLE_LOG(error, "[ntf::gl_check_error] GL ERROR ({}) | \"{}\":{} -> {}",
               err, file, line, err_str);
  }
  return err;
}

static constexpr GLuint GL_NULL_ID = 0;

namespace ntf {

gl_state::gl_state(gl_context& ctx) noexcept :
  _ctx{ctx},
  // _tex_limits{0, 0, 0},
  _active_tex{0},
  _bound_vao{0},
  _bound_program{0} {
  std::memset(_bound_buffers, GL_NULL_ID, BUFFER_TYPE_COUNT*sizeof(GLuint));
  std::memset(_bound_fbos, GL_NULL_ID, FBO_BIND_COUNT*sizeof(GLuint));
}

void gl_state::init() noexcept {
  GLint max_tex;
  GL_CALL(glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_tex));
  _bound_texs.resize(max_tex, std::make_pair(0, 0));

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

auto gl_state::create_buffer(r_buffer_type type, const void* data, size_t size) -> buffer_t {
  GLuint id;
  GL_CALL(glGenBuffers(1, &id));
  const GLenum gltype = buffer_type_cast(type);

  GL_CALL(glBindBuffer(gltype, id));
  _buffer_pos(gltype) = id;

  GLbitfield flags = GL_MAP_READ_BIT | GL_MAP_COHERENT_BIT;
  if (data) {
    GL_CALL(glBufferStorage(gltype, size, data, flags));
    GL_CALL(glBufferSubData(gltype, 0, size, data));
  } else {
    flags |= GL_DYNAMIC_STORAGE_BIT;
    GL_CALL(glBufferStorage(gltype, size, data, flags));
  }

  buffer_t buff;
  buff.id = id;
  buff.size = size;
  buff.type = gltype;
  buff.flags = flags;
  return buff;
}

void gl_state::destroy_buffer(const buffer_t& buffer) noexcept {
  NTF_ASSERT(buffer.id);
  GLuint id = buffer.id;
  GLenum& pos = _buffer_pos(buffer.type);
  if (pos == id) {
    GL_CALL(glBindBuffer(buffer.type, GL_NULL_ID));
    pos = GL_NULL_ID;
  }
  GL_CALL(glDeleteBuffers(1, &id));
}

void gl_state::bind_buffer(const buffer_t& buffer) noexcept {
  NTF_ASSERT(buffer.id);
  GLenum& pos = _buffer_pos(buffer.type);
  if (pos == buffer.id) {
    return;
  }
  GL_CALL(glBindBuffer(buffer.type, buffer.id));
  pos = buffer.id;
}

void gl_state::update_buffer(const buffer_t& buffer, const void* data,
                             size_t size, size_t off) noexcept {
  NTF_ASSERT(buffer.flags & GL_DYNAMIC_STORAGE_BIT);
  NTF_ASSERT(size+off <= buffer.size);

  bind_buffer(buffer);
  GL_CALL(glBufferSubData(buffer.type, off, size, data));
}

auto gl_state::create_vao() noexcept -> vao_t {
  GLuint id;
  GL_CALL(glGenVertexArrays(1, &id));
  vao_t vao;
  vao.id = id;
  return vao;
}

void gl_state::bind_vao(const vao_t& vao) noexcept {
  NTF_ASSERT(vao.id);
  if (_bound_vao == vao.id) {
    return;
  }
  GL_CALL(glBindVertexArray(vao.id));
  _bound_vao = vao.id;
}

void gl_state::destroy_vao(const vao_t& vao) noexcept {
  NTF_ASSERT(vao.id);
  GLuint id = vao.id;
  if (_bound_vao == id) {
    _bound_vao = GL_NULL_ID;
    GL_CALL(glBindVertexArray(GL_NULL_ID));
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
  GL_CALL(glShaderSource(id, 1, &src_data, nullptr));
  GL_CALL(glCompileShader(id));

  int succ;
  GL_CALL(glGetShaderiv(id, GL_COMPILE_STATUS, &succ));
  if (!succ) {
    char msg[1024];
    GL_CALL(glGetShaderInfoLog(id, 1024, nullptr, msg));
    GL_CALL(glDeleteShader(id));
    throw ntf::error<void>{"[ntf::gl_state] Failed to compile shader: {}", msg};
  }

  shader_t shader;
  shader.id = id;
  shader.type = gltype;
  return {shader};
}

void gl_state::destroy_shader(const shader_t& shader) noexcept {
  NTF_ASSERT(shader.id);
  GL_CALL(glDeleteShader(shader.id));
}

auto gl_state::create_program(const shader_t* shaders, uint32 count) -> program_t {
  GLuint id;
  GL_CALL(id = glCreateProgram());
  for (uint32 i = 0; i < count; ++i) {
    // TODO: Ensure vertex and fragment exist
    // TODO: Check for duplicate shader types
    GL_CALL(glAttachShader(id, shaders[i].id));
  }
  GL_CALL(glLinkProgram(id));

  int succ;
  GL_CALL(glGetProgramiv(id, GL_LINK_STATUS, &succ));
  if (!succ) {
    char msg[1024];
    GL_CALL(glGetShaderInfoLog(shaders[0].id, 1024, nullptr, msg));
    GL_CALL(glDeleteProgram(id));
    throw ntf::error<void>{"[ntf::gl_state] Failed to link program: {}", msg};
  }

  program_t prog;
  prog.id = id;
  return {prog};
}

void gl_state::destroy_program(const program_t& prog) noexcept {
  NTF_ASSERT(prog.id);
  if (_bound_program == prog.id) {
    _bound_program = GL_NULL_ID;
    GL_CALL(glUseProgram(GL_NULL_ID));
  }
  GL_CALL(glDeleteProgram(prog.id));
}

void gl_state::bind_program(const program_t& prog) noexcept {
  NTF_ASSERT(prog.id);
  if (_bound_program == prog.id) {
    return;
  }
  GL_CALL(glUseProgram(prog.id));
  _bound_program = prog.id;
}

GLenum gl_state::texture_type_cast(r_texture_type type) noexcept {
  switch (type) {
    case r_texture_type::texture1d:       return GL_TEXTURE_1D;
    case r_texture_type::texture1d_array: return GL_TEXTURE_1D_ARRAY;
    case r_texture_type::texture2d:       return GL_TEXTURE_2D;
    case r_texture_type::texture2d_array: return GL_TEXTURE_2D_ARRAY;
    case r_texture_type::texture3d:       return GL_TEXTURE_3D;
    case r_texture_type::cubemap:         return GL_TEXTURE_CUBE_MAP;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::mono:    return GL_R8;
    case r_texture_format::rgb:     return GL_RGB8;
    case r_texture_format::rgba:    return GL_RGBA8;
  };

  NTF_UNREACHABLE();
}

GLenum gl_state::texture_format_underlying_cast(r_texture_format format) noexcept {
  switch (format) {
    case r_texture_format::mono:  [[fallthrough]];
    case r_texture_format::rgb:   [[fallthrough]];
    case r_texture_format::rgba:  return GL_UNSIGNED_BYTE;
  }

  NTF_UNREACHABLE();
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
                              uvec3 dim, uint32 array_size, uint32 mipmaps) -> texture_t {
  const uint32 level = 1+mipmaps;
  NTF_ASSERT(level <= MAX_TEXTURE_LEVEL);

  const GLenum gltype = texture_type_cast(type);
  const GLenum glformat = texture_format_cast(format);

  GLuint id;
  GL_CALL(glGenTextures(1, &id));

  auto& [active_id, active_type] = _bound_texs[_active_tex];
  // glActiveTexture(GL_TEXTURE0+_tex_active_index);
  GL_CALL(glBindTexture(gltype, id));
  active_id = id;
  active_type = gltype;

  switch (gltype) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(dim.x > 0 && dim.x <= _tex_limits.max_dim);
      NTF_ASSERT(array_size > 0);
      GL_CALL(glTexStorage2D(gltype, level, glformat, dim.x, array_size));
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(dim.x > 0 && dim.x <= _tex_limits.max_dim);
      GL_CALL(glTexStorage1D(gltype, level, glformat, dim.x));
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(dim.x > 0 && dim.x <= _tex_limits.max_dim);
      NTF_ASSERT(dim.y > 0 && dim.y <= _tex_limits.max_dim);
      NTF_ASSERT(array_size > 0);
      GL_CALL(glTexStorage3D(gltype, level, glformat, dim.x, dim.y, array_size));
      break;
    }

    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(dim.x == dim.y);
      [[fallthrough]];
    }
    case GL_TEXTURE_2D: {
      NTF_ASSERT(dim.x > 0 && dim.x <= _tex_limits.max_dim);
      NTF_ASSERT(dim.y > 0 && dim.y <= _tex_limits.max_dim);
      GL_CALL(glTexStorage2D(gltype, level, glformat, dim.x, dim.y));
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(dim.x > 0 && dim.x <= _tex_limits.max_dim3d);
      NTF_ASSERT(dim.y > 0 && dim.y <= _tex_limits.max_dim3d);
      NTF_ASSERT(dim.z > 0 && dim.z <= _tex_limits.max_dim3d);
      GL_CALL(glTexStorage3D(gltype, level, glformat, dim.x, dim.y, dim.z));
      break;
    };

    default: {
      NTF_UNREACHABLE();
      break;
    }
  };

  const GLenum glsamplermin = texture_sampler_cast(sampler, (level > 1));
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
  tex.type = gltype;
  tex.format = glformat;
  tex.sampler[0] = glsamplermin;
  tex.sampler[1] = glsamplermag;
  tex.addressing = gladdress;
  tex.dim = dim;
  tex.level = level;
  tex.array_size = array_size;
  tex.attached_fbos = 0;
  return tex;
}

void gl_state::destroy_texture(const texture_t& tex) noexcept {
  NTF_ASSERT(tex.id);
  NTF_ASSERT(tex.attached_fbos == 0);
  auto& [id, type] = _bound_texs[_active_tex];
  if (id == tex.id) {
    // glActiveTexture(GL_TEXTURE0+_tex_active_index);
    GL_CALL(glBindTexture(type, GL_NULL_ID));
    id = GL_NULL_ID;
    type = GL_NULL_ID;
  }
  GLuint texid = tex.id;
  GL_CALL(glDeleteTextures(1, &texid));
}

void gl_state::bind_texture(const texture_t& tex, uint32 index) noexcept {
  NTF_ASSERT(tex.id);
  NTF_ASSERT(index < _bound_texs.size())
  auto& [id, type] = _bound_texs[index];
  if (id == tex.id) {
    return;
  }
  GL_CALL(glActiveTexture(GL_TEXTURE0+index));
  GL_CALL(glBindTexture(tex.type, tex.id));
  id = tex.id;
  type = tex.type;
}

void gl_state::update_texture_data(const texture_t& tex, const void* data, 
                                   r_texture_format format,
                                   uvec3 offset, uint32 index,
                                   uint32 level, bool gen_mipmaps) noexcept {
  NTF_ASSERT(tex.id);
  NTF_ASSERT(data);
  NTF_ASSERT(level <= tex.level);
  const GLenum data_format = texture_format_cast(format);
  const GLenum data_format_ul = texture_format_underlying_cast(format);

  bind_texture(tex, _active_tex);
  switch (tex.type) {
    case GL_TEXTURE_1D_ARRAY: {
      NTF_ASSERT(offset.x < tex.dim.x);
      NTF_ASSERT(index < tex.array_size);
      GL_CALL(glTexSubImage2D(tex.type, level,
                              offset.x, index,
                              tex.dim.x, tex.array_size,
                              data_format, data_format_ul, data));
      break;
    }

    case GL_TEXTURE_1D: {
      NTF_ASSERT(offset.x < tex.dim.x);
      GL_CALL(glTexSubImage1D(tex.type, level,
                              offset.x, tex.dim.x,
                              data_format, data_format_ul, data));
      break;
    }

    case GL_TEXTURE_2D_ARRAY: {
      NTF_ASSERT(offset.x < tex.dim.x);
      NTF_ASSERT(offset.y < tex.dim.y);
      NTF_ASSERT(index < tex.array_size);
      GL_CALL(glTexSubImage3D(tex.type, level,
                              offset.x, offset.y, index, 
                              tex.dim.x, tex.dim.y, tex.array_size,
                              data_format, data_format_ul, data));
      break;
    }

    case GL_TEXTURE_2D: {
      NTF_ASSERT(offset.x < tex.dim.x);
      NTF_ASSERT(offset.y < tex.dim.y);
      GL_CALL(glTexSubImage2D(tex.type, level,
                              offset.x, offset.y,
                              tex.dim.x, tex.dim.y,
                              data_format, data_format_ul, data));
      break;
    }
    
    case GL_TEXTURE_CUBE_MAP: {
      NTF_ASSERT(offset.x < tex.dim.x);
      NTF_ASSERT(offset.y < tex.dim.y);
      NTF_ASSERT(index < 6);
      GL_CALL(glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+index, level,
                              offset.x, offset.y,
                              tex.dim.x, tex.dim.y,
                              data_format, data_format_ul, data));
      break;
    }

    case GL_TEXTURE_3D: {
      NTF_ASSERT(offset.x < tex.dim.x);
      NTF_ASSERT(offset.y < tex.dim.y);
      NTF_ASSERT(offset.z < tex.dim.z);
      GL_CALL(glTexSubImage3D(tex.type, level,
                              offset.x, offset.y, offset.z,
                              tex.dim.x, tex.dim.y, tex.dim.z,
                              data_format, data_format_ul, data));
      break;
    }

    default: {
      NTF_UNREACHABLE();
      break;
    }
  }

  if (gen_mipmaps) {
    GL_CALL(glGenerateMipmap(tex.type));
  }
}

void gl_state::update_texture_sampler(texture_t& tex, r_texture_sampler sampler) noexcept {
  NTF_ASSERT(tex.id);
  const GLenum glsamplermin = texture_sampler_cast(sampler, (tex.level > 1));
  const GLenum glsamplermag = texture_sampler_cast(sampler, false); // No mipmaps for mag
  if (tex.sampler[0] == glsamplermin && tex.sampler[1] == glsamplermag) {
    return;
  }

  bind_texture(tex, _active_tex);
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

  bind_texture(tex, _active_tex);
  GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_WRAP_S, gladdress)); // U
  if (tex.type != GL_TEXTURE_1D || tex.type != GL_TEXTURE_1D_ARRAY) {
    GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_WRAP_T, gladdress)); // V
    if (tex.type == GL_TEXTURE_3D || tex.type == GL_TEXTURE_CUBE_MAP) {
      GL_CALL(glTexParameteri(tex.type, GL_TEXTURE_WRAP_R, gladdress)); // W (?)
    }
  }
  tex.addressing = gladdress;
}

GLenum gl_state::fbo_attachment_cast(r_clear_flag flags) noexcept {
  const bool depth = +(flags & r_clear_flag::depth);
  const bool stencil = +(flags & r_clear_flag::stencil);
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

auto gl_state::create_framebuffer(uint32 w, uint32 h, r_clear_flag buffers,
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

  GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, GL_NULL_ID));

  NTF_ASSERT(glCheckFramebufferStatus(fbbind) == GL_FRAMEBUFFER_COMPLETE);

  framebuffer_t fbo;
  fbo.sd_rbo = rbos[0];
  fbo.color_rbo = rbos[1];
  fbo.dim.x = w;
  fbo.dim.y = h;
  fbo.clear_flags = 0;
  fbo.viewport.x = 0;
  fbo.viewport.y = 0;
  fbo.viewport.z = w;
  fbo.viewport.w = h;
  fbo.color.r = .3f;
  fbo.color.g = .3f;
  fbo.color.b = .3f;
  fbo.color.a = 1.f;
  return fbo;
}

auto gl_state::create_framebuffer(uint32 w, uint32 h, r_clear_flag buffers,
                                  texture_t** attachments, uint32* levels,
                                  uint32 att_count) -> framebuffer_t {
  NTF_ASSERT(att_count <= MAX_FBO_ATTACHMENTS);
  NTF_ASSERT(attachments);
  NTF_ASSERT(levels);
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
    GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, GL_NULL_ID));
  }

  for (uint32 i = 0; i < att_count; ++i) {
    NTF_ASSERT(attachments[i]);
    auto& tex = *attachments[i];
    bind_texture(tex, _active_tex);
    switch (tex.type) {
      case GL_TEXTURE_1D: {
        NTF_ASSERT(w == tex.dim.x && h == 1);
        GL_CALL(glFramebufferTexture1D(fbbind, GL_COLOR_ATTACHMENT0+i,
                                       tex.type, tex.id, levels[i]));
        break;
      }
      case GL_TEXTURE_2D: {
        NTF_ASSERT(w == tex.dim.x && h == tex.dim.y);
        GL_CALL(glFramebufferTexture2D(fbbind, GL_COLOR_ATTACHMENT0+i,
                                       tex.type, tex.id, levels[i]));
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
  fbo.sd_rbo = rbo;
  fbo.color_rbo = GL_NULL_ID;
  fbo.dim.x = w;
  fbo.dim.y = h;
  fbo.clear_flags = 0;
  fbo.viewport.x = 0;
  fbo.viewport.y = 0;
  fbo.viewport.z = w;
  fbo.viewport.w = h;
  fbo.color.r = .3f;
  fbo.color.g = .3f;
  fbo.color.b = .3f;
  fbo.color.a = 1.f;
  return fbo;
}

void gl_state::destroy_framebuffer(const framebuffer_t& fbo) noexcept {
  NTF_ASSERT(fbo.id);
  if (_bound_fbos[FBO_BIND_WRITE] == fbo.id) {
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NULL_ID));
    _bound_fbos[FBO_BIND_WRITE] = GL_NULL_ID;
  } else if (_bound_fbos[FBO_BIND_READ] == fbo.id) {
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, GL_NULL_ID));
    _bound_fbos[FBO_BIND_READ] = GL_NULL_ID;
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

void gl_state::bind_framebuffer(const framebuffer_t& fbo, fbo_binding binding) noexcept {
  NTF_ASSERT(fbo.id);
  if (binding == FBO_BIND_WRITE) {
    if (_bound_fbos[FBO_BIND_WRITE] == fbo.id) {
      return;
    }
    GL_CALL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo.id));
  } else if (binding == FBO_BIND_READ) {
    if (_bound_fbos[FBO_BIND_READ] == fbo.id) {
      return;
    }
    GL_CALL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo.id));
  } else {
    NTF_UNREACHABLE();
  }
  _bound_fbos[binding] = fbo.id;
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

void gl_context::_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
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
    SHOGLE_LOG(error, "[ntf::gl_context][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    SHOGLE_LOG(verbose, "[ntf::gl_context][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  } else {
    SHOGLE_LOG(debug, "[ntf::gl_context][{}][{}][{}][{}] {}",
               severity_msg, type_msg, src_msg, id, msg);
  }
}

gl_context::gl_context(r_window& win, uint32 major, uint32 minor) :
  _state{*this}, _major{major}, _minor{minor} {
  if (!gladLoadGLLoader(win.proc_loader())) {
    throw ntf::error<void>{"[ntf::gl_context] Failed to load GLAD"};
  }
  _proc_fun = win.proc_loader();
  _viewport = uvec4{0, 0, win.fb_size()};

  GL_CALL(glEnable(GL_DEBUG_OUTPUT));
  GL_CALL(glDebugMessageCallback(gl_context::_debug_callback, this));

  GL_CALL(glViewport(_viewport.x, _viewport.y, _viewport.z, _viewport.w));

  _state.init();
  _vao = _state.create_vao();
  _state.bind_vao(_vao);

  GL_CALL(glEnable(GL_DEPTH_TEST)); // (?)
}

gl_context::~gl_context() noexcept {}

void gl_context::enqueue(r_draw_cmd cmd) {
  auto* alloc_ptr = _cmd_arena.allocate<r_draw_cmd>(1);
  std::construct_at(alloc_ptr, cmd);
  if (cmd.framebuffer) {
    NTF_ASSERT(cmd.framebuffer.api == r_api::opengl);
    auto& cmds = _fb_cmds[cmd.framebuffer.handle]; // May create one
    cmds.emplace_back(alloc_ptr);
  } else {
    auto& cmds = _fb_cmds[r_handle_tombstone]; // default fbo
    cmds.emplace_back(alloc_ptr);
  }
}

void gl_context::start_frame() {
#ifdef SHOGLE_ENABLE_IMGUI
  ImGui::NewFrame();
#endif
  for(auto& [_,cmds] : _fb_cmds) {
    cmds.clear();
  }
  _cmd_arena.reset();
}

void gl_context::end_frame() {
  auto draw_things = [](GLenum prim, uint32 offset, uint32 count, uint32 instances, bool indices) {
    if (!indices) {
      if (instances > 1) {
        glDrawArraysInstanced(prim, offset, count, instances);
      } else {
        glDrawArrays(prim, offset, count);
      }
    } else {
      if (instances > 1) {
        glDrawElementsInstanced(prim, count, GL_UNSIGNED_INT,
                                reinterpret_cast<void*>(offset*sizeof(uint32)), instances);
      } else {
        glDrawElements(prim, count, GL_UNSIGNED_INT,
                       reinterpret_cast<void*>(offset*sizeof(uint32)));
      }
    }
  };

  for (auto& [handle, cmds] : _fb_cmds) {
    GLenum fbo = 0;
    r_clear clear = _glstate.clear_flags;
    uvec4 vp = _glstate.viewport;
    vec4 color = _glstate.clear_color;

    if (handle != r_handle_tombstone) {
      auto& fb = resource<gl_framebuffer>({}, handle);
      fbo = fb._fbo;
      vp = fb.viewport();
      clear = fb.clear_flags();
      color = fb.clear_color();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    _glstate.fbo = fbo;
    glViewport(vp.x, vp.y, vp.z, vp.w);
    gl_clear_bits(clear, color);

    for (auto* cmd_ptr : cmds) {
      auto& cmd = *cmd_ptr;
      bool rebind_attributes = false;

      NTF_ASSERT(cmd.pipeline && cmd.pipeline.api == RENDER_API);
      auto& pipeline = resource<gl_pipeline>({}, cmd.pipeline.handle);
      if (_glstate.program != pipeline._program_id) {
        glUseProgram(pipeline._program_id);
        _glstate.program = pipeline._program_id;
        rebind_attributes = true;
      }

      NTF_ASSERT(cmd.vertex_buffer && cmd.vertex_buffer.api == RENDER_API);
      auto& vbo = resource<gl_buffer>({}, cmd.vertex_buffer.handle);
      if (_glstate.vbo != vbo._id) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo._id);
        _glstate.vbo = vbo._id;
        rebind_attributes = true; // HAS to be reconfigured each time the vertex buffer is rebound
      }

      bool use_indices = bool{cmd.index_buffer};
      if (use_indices) {
        NTF_ASSERT(cmd.index_buffer.api == RENDER_API);
        auto& ebo = resource<gl_buffer>({}, cmd.index_buffer.handle);
        if (_glstate.ebo != ebo._id) {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo._id);
          _glstate.ebo = ebo._id;
        }
      }

      if (rebind_attributes) {
        for (const auto& attrib : pipeline._attribs) {
          const uint32 type_dim = r_attrib_type_dim(attrib.type);
          NTF_ASSERT(type_dim);

          const GLenum gl_underlying_type = gl_attrib_underlying_type_cast(attrib.type);
          NTF_ASSERT(gl_underlying_type);

          glVertexAttribPointer(
            attrib.location,
            type_dim,
            gl_underlying_type,
            GL_FALSE, // Don't normalize,
            pipeline._stride,
            reinterpret_cast<void*>(attrib.offset)
          );
          glEnableVertexAttribArray(attrib.location);
        }
      }

      if (cmd.textures) {
        _glstate.enabled_tex = 0;
        for (uint32 i = 0; i < cmd.texture_count; ++i) {
          auto& tex = resource<gl_texture>({}, cmd.textures[i].handle);
          const GLenum gltype = gl_texture_type_cast(tex.type(), tex.is_array());
          NTF_ASSERT(gltype);

          glActiveTexture(GL_TEXTURE0+i);
          glBindTexture(gltype, tex._id);
          _glstate.enabled_tex |= 1 << i;
        }
      }

      if (cmd.uniforms) {
        for (uint32 i = 0; i < cmd.uniform_count; ++i) {
          auto& unif = cmd.uniforms[i];
          gl_pipeline::push_uniform(unif.location, unif.type, unif.data);
        }
      }

      const GLenum glprim = gl_primitive_cast(cmd.primitive);
      NTF_ASSERT(glprim);
      draw_things(glprim, cmd.draw_offset, cmd.draw_count, cmd.instance_count, use_indices);

    }
  }
  if (_glstate.fbo) {
    auto& vp = _glstate.viewport;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(vp.x, vp.y, vp.z, vp.w);
    _glstate.fbo = 0;
  }

#ifdef SHOGLE_ENABLE_IMGUI
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif

  _swap_buffers();
}

void gl_context::viewport(uint32 x, uint32 y, uint32 w, uint32 h) {
  // glViewport(x, y, w, h);
  _glstate.viewport.x = x;
  _glstate.viewport.y = y;
  _glstate.viewport.z = w;
  _glstate.viewport.w = h;
}

void gl_context::viewport(uint32 w, uint32 h) {
  viewport(_glstate.viewport.x, _glstate.viewport.y, w, h);
}

void gl_context::viewport(uvec2 pos, uvec2 size) {
  viewport(pos.x, pos.y, size.x, size.y);
}

void gl_context::viewport(uvec2 size) {
  viewport(size.x, size.y);
}

void gl_context::clear_color(float32 r, float32 g, float32 b, float32 a) {
  _glstate.clear_color.r = r;
  _glstate.clear_color.g = g;
  _glstate.clear_color.b = b;
  _glstate.clear_color.a = a;
}

void gl_context::clear_color(float32 r, float32 g, float32 b) {
  clear_color(r, g, b, _glstate.clear_color.a);
}

void gl_context::clear_color(color4 color) {
  clear_color(color.r, color.g, color.b, color. a);
}

void gl_context::clear_color(color3 color) {
  clear_color(color.r, color.g, color.b);
}

void gl_context::clear_flags(r_clear clear) {
  _glstate.clear_flags = clear;
}

std::string_view gl_context::name_str() const {
  return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

std::string_view gl_context::vendor_str() const {
  return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

std::string_view gl_context::version_str() const {
  return reinterpret_cast<const char*>(glGetString(GL_VERSION));
}

auto gl_context::make_texture(r_texture_descriptor desc)
                                                      -> expected<texture_handle, gl_texture_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_texture_err>{gl_texture_err::none}; // ?
  }

  r_handle_value handle = _textures.acquire(*this);
  auto& tex = _textures.get(handle);

  tex.load(desc.type, desc.format, desc.sampler, desc.addressing, desc.texels,
           desc.mipmap_level, desc.count, desc.extent);
  if (!tex.complete()) {
    _buffers.push(handle);
    return unexpected<gl_texture_err>{gl_texture_err::none}; // ?
  }

  return texture_handle{*this, handle};
}

auto gl_context::make_buffer(r_buffer_descriptor desc) 
                                                        -> expected<buffer_handle, gl_buffer_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_buffer_err>{gl_buffer_err::none};
  }

  r_handle_value handle = _buffers.acquire(*this);
  auto& buff = _buffers.get(handle);

  buff.load(desc.type, desc.data, desc.size);
  if (!buff.complete()) {
    _buffers.push(handle);
    return unexpected<gl_buffer_err>{gl_buffer_err::none};
  }

  return buffer_handle{*this, handle};
}

auto gl_context::make_pipeline(r_pipeline_descriptor desc) 
                                                    -> expected<pipeline_handle, gl_pipeline_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_pipeline_err>{gl_pipeline_err::none};
  }

  r_handle_value handle = _pipelines.acquire(*this);
  auto& pipeline = _pipelines.get(handle);

  std::vector<gl_shader*> stages(desc.stage_count);
  for (uint32 i = 0; i < desc.stage_count; ++i) {
    stages[i] = &_shaders.get(desc.stages[i].handle);
  }

  pipeline.load(stages.data(), desc.stage_count, desc.attribs, desc.attrib_count);
  if (!pipeline.complete()) {
    _pipelines.push(handle);
    return unexpected<gl_pipeline_err>{gl_pipeline_err::none};
  }

  return pipeline_handle{*this, handle};
}

auto gl_context::make_shader(r_shader_descriptor desc)
                                                        -> expected<shader_handle, gl_shader_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_shader_err>{gl_shader_err::none};
  }

  r_handle_value handle = _shaders.acquire(*this);
  auto& shader = _shaders.get(handle);

  shader.load(desc.type, desc.source);
  if (!shader.complete()) {
    _shaders.push(handle);
    return unexpected<gl_shader_err>{gl_shader_err::none};
  }

  return shader_handle{*this, handle};
}

auto gl_context::make_framebuffer(r_framebuffer_descriptor desc) 
                                              -> expected<framebuffer_handle, gl_framebuffer_err> {
  if (!gl_validate_descriptor(desc)) {
    return unexpected<gl_framebuffer_err>{gl_framebuffer_err::none};
  }

  r_handle_value handle = _framebuffers.acquire(*this);
  auto& fbo = _framebuffers.get(handle);

  const uvec4& vp = desc.viewport;
  fbo.load(vp.x, vp.y, vp.z, vp.w, desc.sampler, desc.addressing);
  if (!fbo.complete()) {
    _framebuffers.push(handle);
    return unexpected<gl_framebuffer_err>{gl_framebuffer_err::none};
  }

  return framebuffer_handle{*this, handle};
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

void gl_pipeline::load(const gl_shader* const* shaders, uint32 shader_count, 
                       const r_attrib_descriptor* attribs, uint32 attrib_count) {
  NTF_ASSERT(!_program_id);

  NTF_ASSERT(shaders && shader_count > 0);

  r_shader_type shader_flags{r_shader_type::none};
  for (uint32 i = 0; i < shader_count; ++i) {
    const auto& shader = *shaders[i];
    NTF_ASSERT(!+(shader_flags & shader._type), "Detected duplicate shader!!!");
    shader_flags &= shader._type;
  }

  size_t stride{0};
  for (uint32 i = 0; i < attrib_count; ++i) {
    r_attrib_descriptor attrib = attribs[i];
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

  _attribs.reserve(attrib_count);
  for (uint32 i = 0; i < attrib_count; ++i) {
    _attribs.emplace_back(attribs[i]);
  }
  _program_id = id;
  _stride = stride;
  _enabled_shaders = shader_flags;
}

void gl_pipeline::unload() {
  NTF_ASSERT(_program_id);

  glDeleteProgram(_program_id);

  _program_id = 0;
  _enabled_shaders = r_shader_type::none;
  _attribs.clear();
  _stride = 0;
}

optional<uint32> gl_pipeline::uniform_location(std::string_view name) const {
  NTF_ASSERT(_program_id);
  const GLint loc = glGetUniformLocation(_program_id, name.data());
  if (loc < 0) {
    return std::nullopt;
  }
  return static_cast<uint32>(loc);
}

void gl_pipeline::push_uniform(uint32 loc, r_attrib_type type, const void* data) {
  switch (type) {
    case r_attrib_type::f32: {
      glUniform1f(loc, *reinterpret_cast<const float32*>(data));
      break;
    }
    case r_attrib_type::vec2: {
      glUniform2fv(loc, 1, glm::value_ptr(*reinterpret_cast<const vec2*>(data)));
      break;
    }
    case r_attrib_type::vec3: {
      glUniform3fv(loc, 1, glm::value_ptr(*reinterpret_cast<const vec3*>(data)));
      break;
    }
    case r_attrib_type::vec4: {
      glUniform4fv(loc, 1, glm::value_ptr(*reinterpret_cast<const vec4*>(data)));
      break;
    }
    case r_attrib_type::mat3: {
      glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const mat3*>(data)));
      break;
    } 
    case r_attrib_type::mat4: {
      glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const mat4*>(data)));
      break;
    }

    case r_attrib_type::f64: {
      glUniform1d(loc, *reinterpret_cast<const float64*>(data));
      break;
    }
    case r_attrib_type::dvec2: {
      glUniform2dv(loc, 1, glm::value_ptr(*reinterpret_cast<const dvec2*>(data)));
      break;
    }
    case r_attrib_type::dvec3: {
      glUniform3dv(loc, 1, glm::value_ptr(*reinterpret_cast<const dvec3*>(data)));
      break;
    }
    case r_attrib_type::dvec4: {
      glUniform4dv(loc, 1, glm::value_ptr(*reinterpret_cast<const dvec4*>(data)));
      break;
    }
    case r_attrib_type::dmat3: {
      glUniformMatrix3dv(loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const dmat3*>(data)));
      break;
    } 
    case r_attrib_type::dmat4: {
      glUniformMatrix4dv(loc, 1, GL_FALSE, glm::value_ptr(*reinterpret_cast<const dmat4*>(data)));
      break;
    }

    case r_attrib_type::i32: {
      glUniform1i(loc, *reinterpret_cast<const int32*>(data));
      break;
    }
    case r_attrib_type::ivec2: {
      glUniform2iv(loc, 1, glm::value_ptr(*reinterpret_cast<const ivec2*>(data)));
      break;
    }
    case r_attrib_type::ivec3: {
      glUniform3iv(loc, 1, glm::value_ptr(*reinterpret_cast<const ivec3*>(data)));
      break;
    }
    case r_attrib_type::ivec4: {
      glUniform4iv(loc, 1, glm::value_ptr(*reinterpret_cast<const ivec4*>(data)));
      break;
    }

    default: {
      NTF_ASSERT(false, "Invalid type tag");
      break;
    }
  };
}

void gl_shader::load(r_shader_type type, std::string_view src) {
  NTF_ASSERT(!_id);

  const char* src_data = src.data();
  const GLenum gltype = gl_shader_type_cast(type);
  NTF_ASSERT(gltype);

  int succ;
  GLuint id = glCreateShader(gltype);
  glShaderSource(id, 1, &src_data, nullptr);
  glCompileShader(id);
  glGetShaderiv(id, GL_COMPILE_STATUS, &succ);
  if (!succ) {
    // TODO: Store (or pass) the log somewhere
    char log[512]; 
    glGetShaderInfoLog(id, 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader] Shader compilation failed (id: {}) -> {}", id, log);
    glDeleteShader(id);
    return;
  }

  _id = id;
  _type = type;
}

void gl_shader::unload() {
  NTF_ASSERT(_id);

  glDeleteShader(_id);

  _id = 0;
  _type = r_shader_type::none;
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
