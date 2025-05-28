#pragma once

#include "../render/pipeline.hpp"
#include "../render/texture.hpp"
#include "../render/meshes.hpp"
#include "../render/framebuffer.hpp"
#include "../render/vertex.hpp"

#include "../assets/font.hpp"

namespace ntf {

struct font_render_rule {
  virtual ~font_render_rule() = default;
  virtual std::pair<r_pipeline, r_shader_buffer> write_uniforms() = 0;
};

template<font_codepoint_type CodeT>
using text_string_view = std::basic_string_view<CodeT, std::char_traits<CodeT>>;

using glyph_meta = std::tuple<weak_ref<const font_glyphs>, weak_ref<const glyph_map>, vec2>;

class text_buffer {
public:
  struct glyph_entry {
    vec2 transf_scale;
    vec2 transf_offset;
    vec2 uv_scale;
    vec2 uv_offset;
  };

public:
  text_buffer() noexcept = default;

public:
  template<font_codepoint_type CodeT>
  std::pair<float, float> append(glyph_meta font,
                                 float pos_x, float pos_y, float scale,
                                 text_string_view<CodeT> str) {
    float x = pos_x, y = pos_y;
    const auto& [glyphs, map, sz] = font;
    for (const auto ch : str) {
      const auto code = static_cast<char32_t>(ch);
      size_t idx = 0u;
      if (auto g_it = map->find(code); g_it != map->end()) {
        idx = static_cast<size_t>(g_it->second);
      }

      const auto& glyph = (*glyphs)[idx];
      if (code == '\n') {
        x = pos_x;
        y -= static_cast<float>(glyph.advance.y)*scale;
        continue;
      }

      _append_glyph(glyph, x, y, scale, sz.x, sz.y);

      x += static_cast<float>(glyph.advance.x)*scale;
    }
    return std::make_pair(x, y);
  }

  template<typename... Args>
  std::pair<float, float> append_fmt(glyph_meta font,
                                     float pos_x, float pos_y, float scale,
                                     fmt::format_string<Args...> fmt, Args&&... args) {
    auto str = fmt::format(fmt, std::forward<Args>(args)...);
    return append(font, pos_x, pos_y, scale, text_string_view<char>{str});
  }

  void clear() { _glyph_cache.clear(); }

private:
  void _append_glyph(const glyph_metrics& glyph,
                     float x, float y, float scale,
                     float atlas_w, float atlas_h);

public:
  cspan<glyph_entry> data() const { return {_glyph_cache.data(), _glyph_cache.size()}; }

private:
  std::vector<glyph_entry> _glyph_cache;
};

class sdf_text_rule final : public font_render_rule {
private:
  struct glyph_props {
    color3 text_color;
    float text_width;

    color3 out_color;
    float out_width;

    float text_edge;
    float out_edge;

    vec2 out_offset;
  };

private:
  sdf_text_rule(renderer_pipeline&& pipeline, renderer_buffer&& uniform_buffer,
                const glyph_props& props);

public:
  static r_expected<sdf_text_rule> create(r_context_view ctx,
                                          const color3& color, float width, float edge,
                                          const color3& outline_color = {0.f, 0.f, 0.f},
                                          const vec2& outline_offset = {0.f, 0.f},
                                          float outline_width = 0.f, float outline_edge = 0.f);

public:
  std::pair<r_pipeline, r_shader_buffer> write_uniforms() override;

public:
  sdf_text_rule& text_color(const color3& color) & {
    _props.text_color = color;
    return *this;
  }

  sdf_text_rule& text_width(float width) & {
    _props.text_width = width;
    return *this;
  }

  sdf_text_rule& text_edge(float edge) & {
    _props.text_edge = edge;
    return *this;
  }

  sdf_text_rule& outline_color(const color3& color) & {
    _props.out_color = color;
    return *this;
  }

  sdf_text_rule& outline_color(float r, float g, float b) & {
    _props.out_color.r = r;
    _props.out_color.g = g;
    _props.out_color.b = b;
    return *this;
  }

  sdf_text_rule& outline_offset(const vec2& offset) & {
    _props.out_offset = offset;
    return *this;
  }

  sdf_text_rule& outline_offset(float x, float y) & {
    _props.out_offset.x = x;
    _props.out_offset.y = y;
    return *this;
  }

  sdf_text_rule& outline_width(float width) & {
    _props.out_width = width;
    return *this;
  }

  sdf_text_rule& outline_edge(float edge) & {
    _props.out_edge = edge;
    return *this;
  }

public:
  color3 text_color() const { return _props.text_color; }
  float text_width() const { return _props.text_width; }
  float text_edge() const { return _props.text_edge; }

  color3 outline_color() const { return _props.out_color; }
  vec2 outline_offset() const { return _props.out_offset; }
  float outline_width() const { return _props.out_width; }
  float outline_edge() const { return _props.out_edge; }

private:
  renderer_pipeline _pipeline;
  renderer_buffer _uniform_buffer;
  glyph_props _props;
};

class bitmap_text_rule final : public font_render_rule {
private:
  bitmap_text_rule(renderer_pipeline&& pipeline, renderer_buffer&& uniform_buffer,
                   const color3& color);

public:
  static r_expected<bitmap_text_rule> create(r_context_view ctx, const color3& color);

public:
  std::pair<r_pipeline, r_shader_buffer> write_uniforms() override;

public:
  bitmap_text_rule& color(const color3& col) & {
    _text_color = col;
    return *this;
  } 

  bitmap_text_rule& color(float r, float g, float b) & {
    _text_color.r = r;
    _text_color.g = g;
    _text_color.b = b;
    return *this;
  }

public:
  color3 color() const { return _text_color; }

private:
  renderer_pipeline _pipeline;
  renderer_buffer _uniform_buffer;
  color3 _text_color;
};

class font_renderer {
private:
  struct ssbo_callback_t {
    r_buffer_view ssbo;
    cspan<text_buffer::glyph_entry> buffer_data;
    size_t glyph_count;
    size_t offset;

    void operator()(r_context ctx) const;
  };

private:
  font_renderer(renderer_buffer&& ssbo, renderer_texture&& atlas,
                font_glyphs&& glyphs, glyph_map&& map,
                const mat4& transform, vec2 bitmap_extent, size_t batch) noexcept;

public:
  static r_expected<font_renderer> create(r_context_view ctx, 
                                          const mat4& transform,
                                          font_atlas_data&& font, 
                                          r_texture_sampler sampler = r_texture_sampler::linear,
                                          size_t batch_size = 64u);

public:
  void clear_state();
  void append_text(const text_buffer& buffer);
  void render(const quad_mesh& quad, r_framebuffer_view fbo,
              font_render_rule& render_rule,
              uint32 sort_group = 0u);
  void render(const quad_mesh& quad, r_framebuffer_view fbo,
              font_render_rule& render_rule, const text_buffer& buffer,
              uint32 sort_group = 0u);

public:
  glyph_meta glyphs() const { return std::make_tuple(&_glyphs, &_glyph_map, _bitmap_extent); }
  
private:
  std::vector<ssbo_callback_t> _write_callbacks;

  renderer_buffer _ssbo;
  renderer_texture _atlas_tex;

  font_glyphs _glyphs;
  glyph_map _glyph_map;

  mat4 _transform;
  vec2 _bitmap_extent;
  size_t _batch_sz;
};

} // namespace ntf
