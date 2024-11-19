#pragma once

#include "./common.hpp"

namespace ntf {

class gl_shader {
public:
  using context_type = gl_context;

public:
  gl_shader() = default;

  gl_shader(GLuint id, shader_category type) :
    _id(id), _type(type) {}

  gl_shader(std::string_view src, shader_category type)
    { load(src, type); }

public:
  void load(std::string_view src, shader_category type);
  void unload();

public:
  shader_category type() const { return _type; }
  bool compiled() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const;
  
  explicit operator bool() const { return compiled(); }

private:
  GLuint _id{0};
  shader_category _type;

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

  gl_shader_program(GLuint id) :
    _id(id) {}

  gl_shader_program(std::string_view vert_src, std::string_view frag_src)
    { load(vert_src, frag_src); }

  gl_shader_program(std::string_view vert_src, std::string_view frag_src,
                    std::string_view geom_src)
    { load(vert_src, frag_src, geom_src); }

  gl_shader_program(shader_type vert, shader_type frag)
    { load(std::move(vert), std::move(frag)); }

  gl_shader_program(shader_type vert, shader_type frag, shader_type geom)
    { load(std::move(vert), std::move(frag), std::move(geom)); }

public:
  void load(std::string_view vert_src, std::string_view frag_src);
  void load(std::string_view vert_src, std::string_view frag_src, std::string_view geom_src);
  void load(shader_type vert, shader_type frag);
  void load(shader_type vert, shader_type frag, shader_type geom);

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
  bool set_uniform(std::string_view name, const T& val) const {
    uniform_type location = uniform_location(name);
    if (!gl_validate_uniform(location)) {
      return false;
    }

    set_uniform(location, val);
    return true;
  }

  uniform_type uniform_location(std::string_view name) const;

public:
  bool linked() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const

  explicit operator bool() const { return linked(); }

private:
  GLuint _id{0};

public:
  NTF_DECLARE_MOVE_ONLY(gl_shader_program);
};

} // namespace ntf
