#pragma once

#include <shogle/render/gl.hpp>

namespace ntf {

class gl_renderer::shader {
public:
  using renderer_type = gl_renderer;

  struct loader {
    shader operator()(std::string_view src, shader_category type) {
      return shader{src, type};
    }
  };

public:
  shader() = default;

  shader(GLuint id, shader_category type) :
    _id(id), _type(type) {}

  shader(std::string_view src, shader_category type);

public:
  void unload();

public:
  shader_category type() const { return _type; }
  bool compiled() const { return _id != 0; }
  GLuint& id() { return _id; } // Not const;
  
  operator bool() const { return compiled(); }

private:
  GLuint _id{0};
  shader_category _type;

public:
  NTF_DECLARE_MOVE_ONLY(shader);
};


class gl_renderer::shader_program {
public:
  using renderer_type = gl_renderer;
  using uniform_type = gl_renderer::shader_uniform;

  struct loader {
    shader_program operator()(std::string_view vert, std::string_view frag) {
      return shader_program{
        shader{vert, shader_category::vertex},
        shader{frag, shader_category::fragment},
      };
    }
    shader_program operator()(std::string_view vert, std::string_view frag, std::string_view geom) {
      return shader_program{
        shader{vert, shader_category::vertex},
        shader{frag, shader_category::fragment},
        shader{geom, shader_category::geometry}
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
  void set_uniform(uniform_type location, const int val) const;
  void set_uniform(uniform_type location, const float val) const;
  void set_uniform(uniform_type location, const vec2& val) const;
  void set_uniform(uniform_type location, const vec3& val) const;
  void set_uniform(uniform_type location, const vec4& val) const;
  void set_uniform(uniform_type location, const mat3& val) const;
  void set_uniform(uniform_type location, const mat4& val) const;

  template<typename T>
  bool set_uniform(std::string_view name, const T& val) const;

  bool uniform_location(uniform_type& uniform, std::string_view name) const;

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

} // namespace ntf

#ifndef SHOGLE_RENDER_SHADER_INL
#include <shogle/render/gl/shader.inl>
#endif
