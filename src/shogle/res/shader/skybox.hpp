#pragma once

#include <shogle/render/shader.hpp>
#include <shogle/render/texture.hpp>
#include <shogle/render/mesh.hpp>

namespace ntf::shogle {

class skybox_shader {
public:
  using att_coords = shader_attribute<0, vec3>;

public:
  skybox_shader();

public:
  inline skybox_shader& set_view(mat4 view) {
    _shader.set_uniform(_view_unif, view);
    return *this;
  }

  inline skybox_shader& set_proj(mat4 proj) {
    _shader.set_uniform(_proj_unif, proj);
    return *this;
  }

  inline skybox_shader& bind_cubemap(const texture2d& tex) {
    render_bind_sampler(tex, (size_t)_cubemap_sampler);
    _shader.set_uniform(_cubemap_unif, (int)_cubemap_sampler);
    return *this;
  }

  inline void draw(const mesh& mesh) {
    render_draw_mesh(mesh);
  }

private:
  enum: int { _cubemap_sampler = 0 };

  shader_program _shader{};
  shader_program::uniform_id _view_unif{}, _proj_unif{};
  shader_program::uniform_id _cubemap_unif{};
};

} // namespace ntf::shogelources
