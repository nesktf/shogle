#pragma once

#include <shogle/render/shader.hpp>
#include <shogle/render/framebuffer.hpp>
#include <shogle/res/spritesheet.hpp>

namespace ntf::shogle {

// Quad shaders
class sprite_shader {
public:
  sprite_shader() = default;

public:
  void compile();
  shader_program& program() { return _shader; } // Not const

public:
  const sprite_shader& enable() const {
    render_use_shader(_shader);
    return *this;
  }

  const sprite_shader& set_transform(const mat4& model) const {
    render_set_uniform(_model_unif, model);
    return *this;
  }

  const sprite_shader& set_view(const mat4& view) const {
    render_set_uniform(_view_unif, view);
    return *this;
  }

  const sprite_shader& set_proj(const mat4& proj) const {
    render_set_uniform(_proj_unif, proj);
    return *this;
  }

  const sprite_shader& set_color(const color4& color) const {
    render_set_uniform(_color_unif, color);
    return *this;
  }

  const sprite_shader& bind_sprite(const sprite& sprite, size_t index) const {
    const auto offset = sprite.tex_offset(index);

    render_set_uniform(_offset_linear_unif, vec2{offset.x, offset.y});
    render_set_uniform(_offset_const_unif, vec2{offset.z, offset.w});

    render_bind_sampler(sprite.tex(), (size_t)_sprite_sampler);
    render_set_uniform(_sampler_unif, (int)_sprite_sampler);
    return *this;
  }

private:
  enum : int { _sprite_sampler = 0 };

  shader_program _shader{};
  shader_uniform _proj_unif{}, _view_unif{}, _model_unif{};
  shader_uniform _offset_linear_unif{}, _offset_const_unif{};
  shader_uniform _color_unif{}, _sampler_unif{};
};


class framebuffer_shader {
public:
  framebuffer_shader() = default;

public:
  void compile();
  shader_program& program() { return _shader; } // Not const

public:
  const framebuffer_shader& enable() const {
    render_use_shader(_shader);
    return *this;
  }

  const framebuffer_shader& set_transform(const mat4& model) const {
    render_set_uniform(_model_unif, model);
    return *this;
  }

  const framebuffer_shader& set_view(const mat4& view) const {
    render_set_uniform(_view_unif, view);
    return *this;
  }

  const framebuffer_shader& set_proj(const mat4& proj) const {
    render_set_uniform(_proj_unif, proj);
    return *this;
  }

  const framebuffer_shader& bind_framebuffer(const framebuffer& fb) const {
    render_bind_sampler(fb.tex(), (size_t)_fb_sampler);
    render_set_uniform(_sampler_unif, (int)_fb_sampler);
    return *this;
  }

private:
  enum: int { _fb_sampler = 0 };

  shader_program _shader{};
  shader_uniform _proj_unif{}, _view_unif{}, _model_unif{};
  shader_uniform _sampler_unif{};
};


class font_shader {
public:
  font_shader() = default;

public:
  void compile();
  shader_program& program() { return _shader; } // Not const

public:
  const font_shader& enable() const {
    render_use_shader(_shader);
    return *this;
  }

  const font_shader& set_proj(const mat4& proj) const {
    render_set_uniform(_proj_unif, proj);
    return *this;
  }

  const font_shader& set_color(const color4& color) const {
    render_set_uniform(_color_unif, color);
    return *this;
  }

  const font_shader& bind_sampler() const {
    render_set_uniform(_sampler_unif, (int)_font_sampler);
    return *this;
  }

private:
  enum: int { _font_sampler = 0 };

  shader_program _shader{};
  shader_uniform _proj_unif{};
  shader_uniform _color_unif{}, _sampler_unif{};
};


// Mesh shaders
class model_shader {
public:
  model_shader() = default;

public:
  void compile();
  shader_program& program() { return _shader; } // Not const

public:
  const model_shader& enable() const {
    render_use_shader(_shader);
    return *this;
  }

  const model_shader& set_transform(const mat4& model) const {
    render_set_uniform(_model_unif, model);
    return *this;
  }

  const model_shader& set_view(const mat4& view) const {
    render_set_uniform(_view_unif, view);
    return *this;
  }

  const model_shader& set_proj(const mat4& proj) const {
    render_set_uniform(_proj_unif, proj);
    return *this;
  }

  const model_shader& set_shiny(const float shiny) const {
    render_set_uniform(_shiny_unif, shiny);
    return *this;
  }

  const model_shader& bind_diffuse(const texture2d& tex) const {
    render_bind_sampler(tex, (size_t)_diffuse_sampler);
    render_set_uniform(_diffuse_sampler_unif, (int)_diffuse_sampler);
    return *this;
  }

  const model_shader& bind_specular(const texture2d& tex) const {
    render_bind_sampler(tex, (size_t)_specular_sampler);
    render_set_uniform(_specular_sampler_unif, (int)_specular_sampler);
    return *this;
  }

private:
  enum : int { _diffuse_sampler = 0, _specular_sampler = 1 };

  shader_program _shader{};
  shader_uniform _proj_unif{}, _view_unif{}, _model_unif{};
  shader_uniform _diffuse_sampler_unif{}, _specular_sampler_unif{}, _shiny_unif{};
};


class skybox_shader {
public:
  skybox_shader() = default;

public:
  void compile();
  shader_program& program() { return _shader; } // Not const

public:
  const skybox_shader& enable() const {
    render_use_shader(_shader);
    return *this;
  }

  const skybox_shader& set_view(const mat4& view) const {
    render_set_uniform(_view_unif, view);
    return *this;
  }
  
  const skybox_shader& set_proj(const mat4& proj) const {
    render_set_uniform(_proj_unif, proj);
    return *this;
  }

  const skybox_shader& bind_cubemap(const cubemap& cmap) const {
    render_bind_sampler(cmap, (size_t)_cubemap_sampler);
    render_set_uniform(_sampler_unif, (int)_cubemap_sampler);
    return *this;
  }

private:
  enum : int { _cubemap_sampler = 0 };
  
  shader_program _shader{};
  shader_uniform _proj_unif{}, _view_unif{};
  shader_uniform _sampler_unif{};
};

} // namespace ntf::shogle
