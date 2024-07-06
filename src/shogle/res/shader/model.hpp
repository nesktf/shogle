#pragma once

#include <shogle/render/shader.hpp>
#include <shogle/render/texture.hpp>
#include <shogle/render/mesh.hpp>

namespace ntf::shogle {

class model_shader {
public:
  model_shader();

public:
  inline model_shader& enable() {
    _shader.enable();
    return *this;
  }

  inline model_shader& set_transform(mat4 model) {
    _shader.set_uniform(_model_unif, model);
    return *this;
  }

  inline model_shader& set_view(mat4 view) {
    _shader.set_uniform(_view_unif, view);
    return *this;
  }

  inline model_shader& set_proj(mat4 proj) {
    _shader.set_uniform(_proj_unif, proj);
    return *this;
  }

  inline model_shader& bind_diffuse(const texture2d& tex) {
    render_bind_sampler(tex, (size_t)_diffuse_sampler);
    _shader.set_uniform(_diffuse_unif, (int)_diffuse_sampler);
    return *this;
  }

  inline model_shader& bind_specular(const texture2d& tex) {
    render_bind_sampler(tex, (size_t)_specular_sampler);
    _shader.set_uniform(_specular_unif, (int)_specular_sampler);
    return *this;
  }

  inline model_shader& set_shiny(float shiny) {
    _shader.set_uniform(_shiny_unif, shiny);
    return *this;
  }

private:
  enum: int { _diffuse_sampler = 0, _specular_sampler = 1 };

  shader_program _shader {};
  shader_program::uniform_id _model_unif{}, _view_unif{}, _proj_unif{};
  shader_program::uniform_id _diffuse_unif{}, _specular_unif{}, _shiny_unif{};
};

} // namespace ntf::shogle
