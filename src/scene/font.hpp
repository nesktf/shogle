#pragma once

#include "../render/pipeline.hpp"
#include "../render/texture.hpp"
#include "../render/meshes.hpp"
#include "../render/framebuffer.hpp"
#include "../render/vertex.hpp"

#include "../assets/font.hpp"

namespace ntf {

struct font_shader_glyph {
  vec2 transf_scale;
  vec2 transf_offset;
  vec2 uv_scale;
  vec2 uv_offset;
};

template<font_codepoint_type CodeT>
using text_string_view = std::basic_string_view<CodeT, std::char_traits<CodeT>>;

class text_buffer_base {
protected:
  text_buffer_base(renderer_pipeline pipeline) noexcept;

public:
  void append(float x, float y, float scale, float atlas_w, float atlas_h,
              const glyph_metrics& glyph);
  void clear();

public:
  r_pipeline_view pipeline() const { return _pipeline; }
  span_view<font_shader_glyph> cache() const { return {_glyph_cache.data(), _glyph_cache.size()}; }

protected:
  std::vector<font_shader_glyph> _glyph_cache;
  renderer_pipeline _pipeline;
};

class text_buffer : public text_buffer_base {
public:
  using uniform_array = std::array<r_push_constant, 3u>;

private:
  text_buffer(renderer_pipeline pipeline, const color3& color,
              r_uniform u_transf, r_uniform u_sampler, r_uniform u_color)noexcept;

public:
  static r_expected<text_buffer> create(r_shader_view fragment, const color3& color);
  static r_expected<text_buffer> create(r_context_view ctx, const color3& color);

public:
  text_buffer& color(const color3& col) & {
    _text_color = col;
    return *this;
  } 

  text_buffer& color(float r, float g, float b) & {
    _text_color.r = r;
    _text_color.g = g;
    _text_color.b = b;
    return *this;
  }

public:
  uniform_array uniforms(const int32& sampler, const mat4& transform) const;

public:
  color3 color() const { return _text_color; }

private:
  color3 _text_color;
  r_uniform _u_transf, _u_sampler;
  r_uniform _u_color;
};

class sdf_text_buffer : public text_buffer_base {
public:
  struct text_props {
    color3 text_color;
    color3 out_color;
    vec2 out_offset;
    float text_width;
    float text_edge;
    float out_width;
    float out_edge;
  };
  using uniform_array = std::array<r_push_constant, 9u>;

private:
  sdf_text_buffer(renderer_pipeline pipeline, text_props props,
                  r_uniform u_transf, r_uniform u_sampler,
                  r_uniform u_color, r_uniform u_width, r_uniform u_edge,
                  r_uniform u_out_color, r_uniform u_out_off,
                  r_uniform u_out_width, r_uniform u_out_edge) noexcept;

public:
  static r_expected<sdf_text_buffer> create(r_shader_view fragment,
                                            const color3& color, float width, float edge);

  static r_expected<sdf_text_buffer> create(r_shader_view fragment,
                                            const color3& color, float width, float edge,
                                            const color3& outline_color,
                                            const vec2& outline_offset,
                                            float outline_width, float outline_edge);

  static r_expected<sdf_text_buffer> create(r_context_view ctx,
                                            const color3& color, float width, float edge);

  static r_expected<sdf_text_buffer> create(r_context_view ctx,
                                            const color3& color, float width, float edge,
                                            const color3& outline_color,
                                            const vec2& outline_offset,
                                            float outline_width, float outline_edge);

private:
  static r_expected<sdf_text_buffer> _make_buffer(renderer_pipeline pipeline, text_props props);

public:
  sdf_text_buffer& text_color(const color3& color) & {
    _props.text_color = color;
    return *this;
  }

  sdf_text_buffer& text_width(float width) & {
    _props.text_width = width;
    return *this;
  }

  sdf_text_buffer& text_edge(float edge) & {
    _props.text_edge = edge;
    return *this;
  }

  sdf_text_buffer& outline_color(const color3& color) & {
    _props.out_color = color;
    return *this;
  }

  sdf_text_buffer& outline_color(float r, float g, float b) & {
    _props.out_color.r = r;
    _props.out_color.g = g;
    _props.out_color.b = b;
    return *this;
  }

  sdf_text_buffer& outline_offset(const vec2& offset) & {
    _props.out_offset = offset;
    return *this;
  }

  sdf_text_buffer& outline_offset(float x, float y) & {
    _props.out_offset.x = x;
    _props.out_offset.y = y;
    return *this;
  }

  sdf_text_buffer& outline_width(float width) & {
    _props.out_width = width;
    return *this;
  }

  sdf_text_buffer& outline_edge(float edge) & {
    _props.out_edge = edge;
    return *this;
  }

public:
  uniform_array uniforms(const int32& sampler, const mat4& transform) const;

  color3 text_color() const { return _props.text_color; }
  float text_width() const { return _props.text_width; }
  float text_edge() const { return _props.text_edge; }

  color3 outline_color() const { return _props.out_color; }
  vec2 outline_offset() const { return _props.out_offset; }
  float outline_width() const { return _props.out_width; }
  float outline_edge() const { return _props.out_edge; }

private:
  text_props _props;
  r_uniform _u_transf, _u_sampler;
  r_uniform _u_color, _u_width, _u_edge;
  r_uniform _u_out_color, _u_out_offset, _u_out_width, _u_out_edge;
};

class font_renderer {
private:
  font_renderer(renderer_buffer ssbo, renderer_texture atlas,
                font_atlas_data::glyphs_t&& glyphs, font_atlas_data::map_t&& glyph_map,
                vec2 bitmap_extent, size_t batch) noexcept;

public:
  static r_expected<font_renderer> create(r_context_view ctx, font_atlas_data&& font, 
                                          r_texture_sampler sampler = r_texture_sampler::linear,
                                          size_t batch_size = 64u);

public:
  void render(const text_buffer& buffer, r_framebuffer_view fbo,
              const quad_mesh& quad, const mat4& transf_mat,
              span_view<r_push_constant> pconsts = {});
  void render(const sdf_text_buffer& buffer, r_framebuffer_view fbo,
              const quad_mesh& quad, const mat4& transf_mat,
              span_view<r_push_constant> pconsts = {});

  template<font_codepoint_type CodeT>
  std::pair<float, float> append(text_buffer_base& buffer,
                                 float pos_x, float pos_y, float scale,
                                 text_string_view<CodeT> str) {
    float x = pos_x, y = pos_y;
    for (const auto ch : str) {
      _append_codepoint(static_cast<char32_t>(ch), buffer, pos_x, pos_y, scale, x, y);
    }
    return std::make_pair(x, y);
  }

  template<typename... Args>
  std::pair<float, float> append_fmt(text_buffer_base& buffer,
                                     float pos_x, float pos_y, float scale,
                                     fmt::format_string<Args...> fmt, Args&&... args) {
    auto str = fmt::format(fmt, std::forward<Args>(args)...);
    return append(buffer, pos_x, pos_y, scale, text_string_view<char>{str});
  }

private:
  void _append_codepoint(char32_t code, text_buffer_base& buffer,
                         float start_x, float start_y, float scale,
                         float& x, float& y);
  
private:
  std::vector<r_push_constant> _pconst_cache;
  renderer_buffer _ssbo;
  renderer_texture _atlas_tex;
  font_atlas_data::glyphs_t _glyphs;
  font_atlas_data::map_t _glyph_map;
  vec2 _bitmap_extent;
  size_t _batch_sz;
};

} // namespace ntf
