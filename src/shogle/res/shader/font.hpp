#pragma once

#include <shogle/render/shader.hpp>
#include <shogle/render/font.hpp>

namespace ntf::shogle {

class font_shader {
public:
  font_shader();

public:
  inline font_shader& enable() {
    _shader.enable();
    return *this;
  }

  inline font_shader& set_proj(const mat4& proj) {
    _shader.set_uniform(_proj_unif, proj);
    return *this;
  }

  inline font_shader& set_color(const color4& color) {
    _shader.set_uniform(_color_unif, color);
    return *this;
  }

  inline font_shader& bind_sampler() {
    _shader.set_uniform(_sampler_unif, 0);
    return *this;
  }

private:
  shader_program _shader{};
  shader_program::uniform_id _proj_unif{};
  shader_program::uniform_id _color_unif{}, _sampler_unif{};
};

} // namespace ntf::shogle
