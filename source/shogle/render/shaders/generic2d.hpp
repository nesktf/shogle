#pragma once

#include <shogle/render/gl/shader_program.hpp>
#include <shogle/render/gl/texture.hpp>

namespace ntf::shogle::shaders {

class generic2d : public gl::shader_program {
public:
  using att_coords    = gl::shader_attribute<0, vec2>;
  using att_texcoords = gl::shader_attribute<1, vec2>;

public:
  generic2d();

public:
  inline generic2d& set_model(mat4 model) {
    set_uniform(_model_unif, model);
    return *this;
  }

  inline generic2d& set_view(mat4 view) {
    set_uniform(_view_unif, view);
    return *this;
  }

  inline generic2d& set_proj(mat4 proj) {
    set_uniform(_proj_unif, proj);
    return *this;
  }

  inline generic2d& bind_texture(gl::texture& tex) {
    tex.bind(_sprite_sampler);
    set_uniform(_texture_unif, _sprite_sampler);
    return *this;
  }

  inline generic2d& set_linear_offset(vec2 offset) {
    set_uniform(_offset_linear_unif, offset);
    return *this;
  }

  inline generic2d& set_const_offset(vec2 offset) {
    set_uniform(_offset_const_unif, offset);
    return *this;
  }

  inline generic2d& set_color(color4 color) {
    set_uniform(_color_unif, color);
    return *this;
  }

private:
  enum: int { _sprite_sampler = 0 };

  uniform_id _model_unif{}, _view_unif{}, _proj_unif{};
  uniform_id _texture_unif{}, _color_unif{};
  uniform_id _offset_linear_unif{}, _offset_const_unif{};
};

} // namespace ntf::shogle::shaders
