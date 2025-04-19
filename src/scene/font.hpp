#pragma once

#include "../assets/font.hpp"
#include "../render/pipeline.hpp"
#include "../render/texture.hpp"
#include "../render/buffer.hpp"
#include "../render/framebuffer.hpp"
#include "../render/vertex.hpp"
#include "render/transform.hpp"

namespace ntf {

struct font_shader_glyph {
  vec2 transf_scale;
  vec2 transf_offset;
  vec2 uv_scale;
  vec2 uv_offset;
};

using font_render_cache = std::vector<font_shader_glyph>;

class font_renderer {
private:
  font_renderer(renderer_pipeline pip, renderer_buffer ssbo, renderer_texture atlas,
                font_atlas_data&& font, size_t batch);

public:
  static asset_expected<font_renderer> create(r_context_view ctx,
                                              std::string_view vert_src, std::string_view frag_src,
                                              font_atlas_data&& font, size_t batch_size);

public:
  void render(const font_render_cache& cache, r_framebuffer_view fbo,
              r_buffer_view vertex, r_buffer_view index, const mat4& proj);

  template<font_codepoint_type CodeT>
  std::pair<float32, float32> append(font_render_cache& cache,
                                     float pos_x, float pos_y, float scale,
                                     std::basic_string_view<CodeT, std::char_traits<CodeT>> str) {
    float x = pos_x, y = pos_y;
    for (const auto ch : str) {
      _append_codepoint(static_cast<char32_t>(ch), cache, pos_x, pos_y, scale, x, y);
    }
    return std::make_pair(x, y);
  }

private:
  void _append_codepoint(char32_t code, font_render_cache& cache,
                         float start_x, float start_y, float scale,
                         float& x, float& y);
  
public:
  renderer_pipeline _pipeline;
  renderer_buffer _ssbo;
  renderer_texture _atlas_tex;
  font_atlas_data::glyphs_t _glyphs;
  font_atlas_data::map_t _glyph_map;
  extent2d _bitmap_extent;
  size_t _batch_sz;
};

} // namespace ntf
