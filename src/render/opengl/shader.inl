#define SHOGLE_RENDER_OPENGL_SHADER_INL
#include "./shader.hpp"
#undef SHOGLE_RENDER_OPENGL_SHADER_INL

namespace ntf {

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
gl_shader_program::gl_shader_program(Vert&& vert, Frag&& frag) {
  _link(std::forward<Vert>(vert), std::forward<Frag>(frag));
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
  is_forwarding<gl_shader> Geom>
gl_shader_program::gl_shader_program(Vert&& vert, Frag&& frag, Geom&& geom) {
  _link(std::forward<Vert>(vert), std::forward<Frag>(frag), std::forward<Geom>(geom));
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
gl_shader_program& gl_shader_program::link(Vert&& vert, Frag&& frag) & {
  _link(std::forward<Vert>(vert), std::forward<Frag>(frag));
  return *this;
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
gl_shader_program&& gl_shader_program::link(Vert&& vert, Frag&& frag) && {
  _link(std::forward<Vert>(vert), std::forward<Frag>(frag));
  return std::move(*this);
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
  is_forwarding<gl_shader> Geom>
gl_shader_program& gl_shader_program::link(Vert&& vert, Frag&& frag, Geom&& geom) & {
  _link(std::forward<Vert>(vert), std::forward<Frag>(frag), std::forward<Geom>(geom));
  return *this;
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
  is_forwarding<gl_shader> Geom>
gl_shader_program&& gl_shader_program::link(Vert&& vert, Frag&& frag, Geom&& geom) && {
  _link(std::forward<Vert>(vert), std::forward<Frag>(frag), std::forward<Geom>(geom));
  return std::move(*this);
}


gl_shader_program& gl_shader_program::link(std::string_view vert_src,
                                           std::string_view frag_src) & {
  _link(vert_src, frag_src);
  return *this;
}

gl_shader_program&& gl_shader_program::link(std::string_view vert_src,
                                           std::string_view frag_src) && {
  _link(vert_src, frag_src);
  return std::move(*this);
}

gl_shader_program& gl_shader_program::link(std::string_view vert_src,
                                           std::string_view frag_src,
                                           std::string_view geom_src) & {
  _link(vert_src, frag_src, geom_src);
  return *this;
}

gl_shader_program&& gl_shader_program::link(std::string_view vert_src,
                                           std::string_view frag_src,
                                            std::string_view geom_src) && {
  _link(vert_src, frag_src, geom_src);
  return std::move(*this);
}

template<typename T>
requires(uniform_traits<T>::is_uniform)
bool gl_shader_program::set_uniform(std::string_view name, const T& val) const {
  uniform_type location = uniform_location(name);
  if (!gl_validate_uniform(location)) {
    return false;
  }

  set_uniform(location, val);
  return true;
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
void gl_shader_program::_link(Vert&& vert, Frag&& frag) {
  NTF_ASSERT(vert.compiled() && vert.type() == shader_category::vertex,
             "Invalid vertex shader");
  NTF_ASSERT(frag.compiled() && frag.type() == shader_category::fragment,
             "Invalid fragment shader");

  int succ;
  char log[512];
  GLuint id = glCreateProgram();
  glAttachShader(id, vert.id());
  glAttachShader(id, frag.id());
  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert.id(), 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader_program] Shader program link failed (id: {}) -> {}",
               id, log);
    glDeleteProgram(id);
    return;
  }

  if (_id) {
    SHOGLE_LOG(warning, "[ntf::gl_shader_program] Shader program overwritten ({} -> {})", _id, id);
    glDeleteShader(_id);
  } else {
    SHOGLE_LOG(verbose, "[ntf::gl_shader_program] Shader program linked (id: {})", id);
  }
  _id = id;
}

template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
  is_forwarding<gl_shader> Geom>
void gl_shader_program::_link(Vert&& vert, Frag&& frag, Geom&& geom) {
  NTF_ASSERT(vert.compiled() && vert.type() == shader_category::vertex,
             "Invalid vertex shader");
  NTF_ASSERT(frag.compiled() && frag.type() == shader_category::fragment,
             "Invalid fragment shader");
  NTF_ASSERT(geom.compiled() && geom.type() == shader_category::geometry,
             "Invalid geometry shader");

  int succ;
  char log[512];
  GLuint id = glCreateProgram();
  glAttachShader(id, vert.id());
  glAttachShader(id, frag.id());
  glAttachShader(id, geom.id());
  glLinkProgram(id);
  glGetProgramiv(id, GL_LINK_STATUS, &succ);
  if (!succ) {
    glGetShaderInfoLog(vert.id(), 512, nullptr, log);
    SHOGLE_LOG(error, "[ntf::gl_shader_program] Shader program link failed (id: {}) -> {}",
               id, log);
    glDeleteProgram(id);
    return;
  }

  if (_id) {
    SHOGLE_LOG(warning, "[ntf::gl_shader_program] Shader program overwritten ({} -> {})", _id, id);
    glDeleteShader(_id);
  } else {
    SHOGLE_LOG(verbose, "[ntf::gl_shader_program] Shader program linked (id: {})", id);
  }
  _id = id;
}

} // namespace ntf
