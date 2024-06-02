#pragma once

#include <shogle/render/gl/shader_program.hpp>
#include <shogle/render/gl/texture.hpp>

namespace ntf::shogle::shaders {

class generic_skybox : public gl::shader_program {
public:
  using att_coords = gl::shader_attribute<0, vec3>;

public:
  generic_skybox();

public:
  inline generic_skybox& set_view(mat4 view) {
    set_uniform(_view_unif, view);
    return *this;
  }

  inline generic_skybox& set_proj(mat4 proj) {
    set_uniform(_proj_unif, proj);
    return *this;
  }

  inline generic_skybox& bind_cubemap(const gl::texture& tex) {
    tex.bind(_cubemap_sampler);
    set_uniform(_cubemap_unif, _cubemap_sampler);
    return *this;
  }

private:
  enum: int { _cubemap_sampler = 0 };

  uniform_id _view_unif{}, _proj_unif{};
  uniform_id _cubemap_unif{};
};

} // namespace ntf::shogel::shaders
