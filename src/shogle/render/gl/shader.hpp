#pragma once

#include <shogle/render/gl/render.hpp>

namespace ntf::shogle::gl {

class shader_program;

class shader {
private:
  friend shader_program;

public:
  enum class type {
    vertex,
    geometry,
    fragment
  };

public:
  shader(std::string src, type type);

public:
  void compile();

public:
  bool compiled() { return _shad_id != 0; }

public:
  ~shader();
  shader(shader&&) noexcept;
  shader(const shader&) = delete;
  shader& operator=(shader&&) noexcept;
  shader& operator=(const shader&) = delete;

private:
  std::string _src;
  GLuint _shad_id {0};
  type _type;
};

} // namespace ntf::shogle::gl
