#include "./pipeline.hpp"

#include <array>

namespace ntf {

void gl_pipeline::load(const gl_shader** shaders, uint32 shader_count, r_resource_handle attrib) {
  NTF_ASSERT(!_program_id);

  NTF_ASSERT(shaders && shader_count > 0);

  r_shader_type shader_flags{r_shader_type::none};
  for (uint32 i = 0; i < shader_count; ++i) {
    const auto& shader = *shaders[i];
    NTF_ASSERT(!+(shader_flags & shader._type), "Detected duplicate shader!!!");
    shader_flags &= shader._type;
  }

  // size_t stride{0};
  // for (uint32 i = 0; i < attrib_count; ++i) {
  //   r_attrib_info attrib = attribs[i];
  //   stride += r_attrib_type_size(attrib.type);
  // }

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
  _attrib_handle = attrib;
  _enabled_shaders = shader_flags;
}

void gl_pipeline::unload() {
  NTF_ASSERT(_program_id);

  glDeleteProgram(_program_id);

  _program_id = 0;
  _enabled_shaders = r_shader_type::none;
  _attrib_handle = r_resource_tombstone;
}

std::optional<uint32> gl_pipeline::uniform_location(std::string_view name) const {
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

} // namespace ntf
