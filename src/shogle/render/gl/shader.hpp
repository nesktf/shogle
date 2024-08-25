#pragma once

#include <shogle/render/gl.hpp>

namespace ntf {

class gl::shader {
public:
  using renderer = gl;

  struct loader {
    shader operator()(std::string_view src, shader_type type) {
      return shader{src, type};
    }
  };

public:
  shader() = default;

  shader(GLuint id, shader_type type) :
    _id(id), _type(type) {}

  shader(std::string_view src, shader_type type);

public:
  void unload();

public:
  shader_type type() const { return _type; }
  bool compiled() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const;
  
  operator bool() const { return compiled(); }

private:
  GLuint _id{0};
  shader_type _type;

public:
  NTF_DECLARE_MOVE_ONLY(shader);
};


class gl::shader_program {
public:
  using renderer = gl;

  struct loader {
    shader_program operator()(std::string_view vert, std::string_view frag) {
      return shader_program{
        shader{vert, shader_type::vertex},
        shader{frag, shader_type::fragment},
      };
    }
    shader_program operator()(std::string_view vert, std::string_view frag, std::string_view geom) {
      return shader_program{
        shader{vert, shader_type::vertex},
        shader{frag, shader_type::fragment},
        shader{geom, shader_type::geometry}
      };
    }
  };

public:
  shader_program() = default;

  shader_program(GLuint id) :
    _id(id) {}

  shader_program(shader vert, shader frag);
  shader_program(shader vert, shader frag, shader geom);

public:
  void use() const;
  void set_uniform(shader_uniform location, const int val) const;
  void set_uniform(shader_uniform location, const float val) const;
  void set_uniform(shader_uniform location, const vec2& val) const;
  void set_uniform(shader_uniform location, const vec3& val) const;
  void set_uniform(shader_uniform location, const vec4& val) const;
  void set_uniform(shader_uniform location, const mat3& val) const;
  void set_uniform(shader_uniform location, const mat4& val) const;

  template<typename T>
  bool set_uniform(std::string_view name, const T& val) const;

  bool uniform_location(shader_uniform& uniform, std::string_view name) const;

public:
  void unload();

public:
  bool linked() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const

  operator bool() const { return linked(); }

private:
  GLuint _id{0};

public:
  NTF_DECLARE_MOVE_ONLY(shader_program);
};


class gl::uniform_list {
public:
  using renderer = gl;

  struct header {
    shader_uniform uniform;
    uniform_type type;
  };

public:
  uniform_list() = default;

  uniform_list(std::initializer_list<header> uniforms) :
    _uniforms(uniforms) {}

public:
  bool register_uniform(const shader_program& shader, std::string_view uniform, uniform_type type);
  void register_uniform(shader_uniform uniform, uniform_type type);
  void clear();

public:
  const std::vector<header>& uniforms() const { return _uniforms; }

private:
  std::vector<header> _uniforms;
};


class gl::shader_args {
public:
  using renderer = gl;

public:
  shader_args() = default;

  shader_args(const uniform_list& list);

public:
  void bind(const shader_program& shader) const;
  void clear();

  template<typename T>
  bool set_uniform(shader_uniform uniform, T&& val);

  template<typename T>
  bool set_uniform(const shader_program& shader, std::string_view uniform, T&& val);

public:
  size_t size() const { return _uniforms.size(); }
  bool empty() const { return size() == 0; }

  operator bool() const { return !empty(); }

private:
  void bind_uniform(const shader_program& shader, shader_uniform uniform, uniform_type type, size_t off) const;

private:
  std::unordered_map<shader_uniform, std::pair<uniform_type, size_t>> _uniforms;
  uint8_t* _data;

public:
  NTF_DECLARE_MOVE_ONLY(shader_args);
};

} // namespace ntf

#ifndef SHOGLE_RENDER_SHADER_INL
#include <shogle/render/gl/shader.inl>
#endif
