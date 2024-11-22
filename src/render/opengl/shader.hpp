#pragma once

#include "./common.hpp"

namespace ntf {

class gl_shader {
public:
  using context_type = gl_context;

public:
  gl_shader() = default;

  gl_shader(std::string_view src, shader_category type);

public:
  gl_shader& compile(std::string_view src, shader_category type) &;
  gl_shader&& compile(std::string_view src, shader_category type) &&;

  void unload();

public:
  GLuint id() const { return _id; }
  
  bool compiled() const { return _id != 0; }
  shader_category type() const { return _type; }
  
  explicit operator bool() const { return compiled(); }

private:
  void _compile(std::string_view src, shader_category type);
  void _reset();

private:
  GLuint _id{0};
  shader_category _type{shader_category::none};

public:
  NTF_DECLARE_MOVE_ONLY(gl_shader);
};


class gl_shader_program {
public:
  using context_type = gl_context;
  using shader_type = gl_shader;
  using uniform_type = gl_shader_uniform;

public:
  gl_shader_program() = default;

  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
  gl_shader_program(Vert&& vert, Frag&& frag);

  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
    is_forwarding<gl_shader> Geom>
  gl_shader_program(Vert&& vert, Frag&& frag, Geom&& geom);

  gl_shader_program(std::string_view vert_src, std::string_view frag_src);
  gl_shader_program(std::string_view vert_src, std::string_view frag_src,
                    std::string_view geom_src);

public:
  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
  gl_shader_program& link(Vert&& vert, Frag&& frag) &;

  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
  gl_shader_program&& link(Vert&& vert, Frag&& frag) &&;

  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
    is_forwarding<gl_shader> Geom>
  gl_shader_program& link(Vert&& vert, Frag&& frag, Geom&& geom) &;

  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
    is_forwarding<gl_shader> Geom>
  gl_shader_program&& link(Vert&& vert, Frag&& frag, Geom&& geom) &&;

  gl_shader_program& link(std::string_view vert_src, std::string_view frag_src) &;
  gl_shader_program&& link(std::string_view vert_src, std::string_view frag_src) &&;

  gl_shader_program& link(std::string_view vert_src, std::string_view frag_src,
                          std::string_view geom_src) &;
  gl_shader_program&& link(std::string_view vert_src, std::string_view frag_src,
                          std::string_view geom_src) &&;

  void unload();

public:
  void use() const;
  void set_uniform(uniform_type location, const int val) const;
  void set_uniform(uniform_type location, const float val) const;
  void set_uniform(uniform_type location, const vec2& val) const;
  void set_uniform(uniform_type location, const vec3& val) const;
  void set_uniform(uniform_type location, const vec4& val) const;
  void set_uniform(uniform_type location, const mat3& val) const;
  void set_uniform(uniform_type location, const mat4& val) const;

  template<typename T>
  requires(uniform_traits<T>::is_uniform)
  bool set_uniform(std::string_view name, const T& val) const;

  uniform_type uniform_location(std::string_view name) const;

public:
  GLuint id() const { return _id; }
  bool linked() const { return _id != 0; }

  explicit operator bool() const { return linked(); }

private:
  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag>
  void _link(Vert&& vert, Frag&& frag);

  template<is_forwarding<gl_shader> Vert, is_forwarding<gl_shader> Frag,
    is_forwarding<gl_shader> Geom>
  void _link(Vert&& vert, Frag&& frag, Geom&& geom);

  void _link(std::string_view vert_src, std::string_view frag_src);
  void _link(std::string_view vert_src, std::string_view frag_src, std::string_view geom_src);

  void _reset();

private:
  GLuint _id{0};

public:
  NTF_DECLARE_MOVE_ONLY(gl_shader_program);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_OPENGL_SHADER_INL
#include "./shader.inl"
#endif
