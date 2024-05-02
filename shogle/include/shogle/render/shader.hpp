#pragma once

#include <shogle/render/gl.hpp>

namespace ntf::render {

class shader {
public:
  using renderer = gl;
  using loader_t = res::shader_loader;

public:
  shader(loader_t loader);

public:
  inline void use(void) { renderer::shader_enable(_shader); }

  template<typename T>
  inline void set_uniform(const char* name, T val) {
    renderer::shader_unif(_shader, name, val);
  }

private:
  renderer::shader _shader;
};

} // namespace ntf::render
