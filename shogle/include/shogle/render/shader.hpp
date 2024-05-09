#pragma once

#include <shogle/render/gl.hpp>

namespace ntf::render {

struct shader {
public:
  using loader_t = res::shader_loader;

public:
  shader(std::string path);
  shader(loader_t loader);

public:
  inline void use(void) { gl::shader_enable(_shader); }

  template<typename T>
  inline void set_uniform(const char* name, T val) {
    gl::shader_unif(_shader, name, val);
  }

public:
  gl::shader _shader;

public:
  ~shader();
  shader(shader&&) = default;
  shader(const shader&) = delete;
  shader& operator=(shader&&) = default;
  shader& operator=(const shader&) = delete;
};

} // namespace ntf::render
