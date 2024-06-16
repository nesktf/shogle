#pragma once

#include <shogle/render/gl/shader_program.hpp>
#include <shogle/render/gl/texture.hpp>

namespace ntf::shogle::res {

class model_shader : public gl::shader_program {
public:
  using att_coords    = gl::shader_attribute<0, vec3>;
  using att_normals   = gl::shader_attribute<1, vec3>;
  using att_texcoords = gl::shader_attribute<2, vec2>;

public:
  model_shader();

public:
  inline model_shader& set_model(mat4 model) {
    set_uniform(_model_unif, model);
    return *this;
  }

  inline model_shader& set_view(mat4 view) {
    set_uniform(_view_unif, view);
    return *this;
  }

  inline model_shader& set_proj(mat4 proj) {
    set_uniform(_proj_unif, proj);
    return *this;
  }

  inline model_shader& bind_diffuse(const gl::texture& tex) {
    tex.bind(_diffuse_sampler);
    set_uniform(_diffuse_unif, (int)_diffuse_sampler);
    return *this;
  }

  inline model_shader& bind_specular(const gl::texture& tex) {
    tex.bind(_specular_sampler);
    set_uniform(_specular_unif, (int)_specular_sampler);
    return *this;
  }

  inline model_shader& set_shiny(float shiny) {
    set_uniform(_shiny_unif, shiny);
    return *this;
  }

private:
  enum: int { _diffuse_sampler = 0, _specular_sampler = 1 };

  uniform_id _model_unif{}, _view_unif{}, _proj_unif{};
  uniform_id _diffuse_unif{}, _specular_unif{}, _shiny_unif{};
};

} // namespace ntf::shogle::res
