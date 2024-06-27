#pragma once

#include <shogle/render/shader.hpp>
#include <shogle/render/texture.hpp>
#include <shogle/render/mesh.hpp>

namespace ntf::shogle {

class sprite_shader {
public:
  using att_coords    = shader_attribute<0, vec2>;
  using att_texcoords = shader_attribute<1, vec2>;

public:
  sprite_shader();

public:
  inline sprite_shader& set_transform(const mat4& model) {
    _shader.set_uniform(_model_unif, model);
    return *this;
  }

  inline sprite_shader& set_view(const mat4& view) {
    _shader.set_uniform(_view_unif, view);
    return *this;
  }

  inline sprite_shader& set_proj(const mat4& proj) {
    _shader.set_uniform(_proj_unif, proj);
    return *this;
  }

  inline sprite_shader& bind_texture(const texture2d& tex) {
    render_bind_sampler(tex, (size_t)_sprite_sampler);
    _shader.set_uniform(_texture_unif, (int)_sprite_sampler);
    return *this;
  }

  inline sprite_shader& set_tex_offset(const vec4& offset) {
    _shader.set_uniform(_offset_linear_unif, vec2{offset.x, offset.y});
    _shader.set_uniform(_offset_const_unif, vec2{offset.z, offset.w});
    return *this;
  }

  inline sprite_shader& set_color(const color4& color) {
    _shader.set_uniform(_color_unif, color);
    return *this;
  }

  inline void draw(const mesh& mesh) {
    render_draw_mesh(mesh);
  }

private:
  enum: int { _sprite_sampler = 0 };

  shader_program _shader{};
  shader_program::uniform_id _model_unif{}, _view_unif{}, _proj_unif{};
  shader_program::uniform_id _texture_unif{}, _color_unif{};
  shader_program::uniform_id _offset_linear_unif{}, _offset_const_unif{};
};

} // namespace ntf::shogle
