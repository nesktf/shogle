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
static_assert(sizeof(font_shader_glyph)==8*sizeof(float));

using font_render_cache = std::vector<font_shader_glyph>;

using f32 = float32;

class font_renderer {
private:
  font_renderer(renderer_pipeline pip, renderer_buffer ssbo, renderer_texture atlas,
                font_atlas_data<char> font, void* ssbo_mapped, size_t batch) :
    _pipeline{std::move(pip)}, _ssbo{std::move(ssbo)},
    _atlas_tex{std::move(atlas)}, _font{std::move(font)},
    _ssbo_mapped{ssbo_mapped}, _batch_sz{batch} {}

public:
  static auto create(
    r_context_view ctx, std::string_view vert_src, std::string_view frag_src,
    font_atlas_data<char> font, size_t batch_size
  ) -> font_renderer
  {
    const size_t ssbo_sz = batch_size * sizeof(font_shader_glyph);

    auto vert = renderer_shader::create(ctx, {
      .type = ntf::r_shader_type::vertex,
      .source = {vert_src},
    });
    auto frag = renderer_shader::create(ctx, {
      .type = ntf::r_shader_type::fragment,
      .source = {frag_src},
    });

    SHOGLE_LOG(debug, "ACA {} {}", vert.has_value(), frag.has_value());
    auto attr_bind = pnt_vertex::attrib_binding();
    auto attr_desc = pnt_vertex::attrib_descriptor();
    ntf::r_shader shads[] = {vert->handle(), frag->handle()};
    auto pipeline = renderer_pipeline::create(ctx, {
      .stages = {&shads[0], std::size(shads)},
      .attrib_binding = attr_bind,
      .attrib_desc = {attr_desc},
      .primitive = ntf::r_primitive::triangles,
      .poly_mode = ntf::r_polygon_mode::fill,
      .front_face = ntf::r_front_face::clockwise,
      .cull_mode = ntf::r_cull_mode::front_back,
      .tests = ntf::r_pipeline_test::all,
      .depth_compare_op = ntf::r_compare_op::less,
      .stencil_compare_op = ntf::r_compare_op::less,
    });

    SHOGLE_LOG(debug, "ACA2 {}", pipeline.has_value());
    auto ssbo = renderer_buffer::create(ctx, {
      .type = ntf::r_buffer_type::shader_storage,
      .flags = ntf::r_buffer_flag::dynamic_storage | ntf::r_buffer_flag::rw_mappable,
      .size = ssbo_sz,
      .data = nullptr,
    });
    SHOGLE_LOG(debug, "ACA3 {}", ssbo.has_value());
    SHOGLE_LOG(debug, "SZ: {}", ssbo_sz);
    void* mapped = ssbo->map(0u, ssbo_sz).value();

    auto font_desc = font.make_bitmap_descriptor();
    auto tex = renderer_texture::create(ctx, {
      .type = ntf::r_texture_type::texture2d,
      .format = font.bitmap_format,
      .extent = ntf::tex_extent_cast(font.bitmap_extent),
      .layers = 1,
      .levels = 1,
      .images = {font_desc},
      .gen_mipmaps = false,
      .sampler = ntf::r_texture_sampler::linear,
      .addressing = ntf::r_texture_address::repeat,
    });
    SHOGLE_LOG(debug, "ACA4 {}", tex.has_value());

    return font_renderer{
      std::move(*pipeline), std::move(*ssbo), std::move(*tex), std::move(font),
      mapped, batch_size
    };
  }

public:
  auto append(font_render_cache& cache, float32 pos_x, float32 pos_y, float32 scale,
              std::string_view str) -> std::pair<float32, float32> {
    float32 x = pos_x, y = pos_y;
    const f32 tex_x = _font.bitmap_extent.x;
    const f32 tex_y = _font.bitmap_extent.y;
    for (const auto ch : str) {
      size_t idx = 0u;
      if (auto g_it = _font.glyph_map.find(ch); g_it != _font.glyph_map.end()) {
        idx = static_cast<size_t>(g_it->second);
      }
      const auto& glyph = _font.glyphs[idx];
      // SHOGLE_LOG(debug, "'{}', idx: {} -> {}", ch, idx, glyph.id);
      // SHOGLE_LOG(debug, "\t-> sz: {}x{} - off: {},{} - be: {},{} - adv: {},{}",
      //            glyph.size.x, glyph.size.y,
      //            glyph.offset.x, glyph.offset.y,
      //            glyph.bearing.x, glyph.bearing.y,
      //            glyph.advance.x, glyph.advance.y);
      if (ch == '\n') {
        x = pos_x;
        y -= glyph.advance.y*scale;
        continue;
      }

      const f32 xpos = x + ((float)glyph.size.x*.5f+glyph.bearing.x)*scale;
      const f32 ypos = y - ((float)glyph.size.y*.5f-(float)glyph.bearing.y)*scale;
      const f32 w = (float)glyph.size.x*scale;
      const f32 h = (float)glyph.size.y*scale;

      cache.emplace_back(
        vec2{w, h}, vec2{xpos, ypos},
        vec2{w/(tex_x*scale), h/(tex_y*scale)},
        vec2{(glyph.offset.x)/tex_x, (glyph.offset.y)/tex_y}
      );
      
      x += glyph.advance.x*scale;
    }
    return std::make_pair(x, y);
  }

public:
  void render(const font_render_cache& cache, r_framebuffer_view fbo,
              r_buffer_view vertex, r_buffer_view index, const mat4& proj) {
    const size_t batches = (cache.size()/_batch_sz)+1;
    auto ctx = _pipeline.context();

    const r_buffer_binding buf_binds[] = {
      {vertex.handle(), r_buffer_type::vertex, nullopt},
      {index.handle(), r_buffer_type::index, nullopt},
      {_ssbo.handle(), r_buffer_type::shader_storage, 1},
    };
    const r_texture_binding tex_binds[] = {
      {.texture = _atlas_tex.handle(), .location = 0},
    };

    const ntf::r_push_constant pconsts[] = {
      // r_format_pushconst(_pipeline.uniform("model").value(), coso.world()),
      // r_format_pushconst(_pipeline.uniform("view").value(), view),
      r_format_pushconst(_pipeline.uniform("proj").value(), proj),
      r_format_pushconst(_pipeline.uniform("text_color").value(), color4{1.f, 0.f, 0.f, 1.f}),
      r_format_pushconst(_pipeline.uniform("sampler0").value(), 0),
    };

    for (size_t i = 0; i < batches; ++i) {
      r_draw_opts opts;
      opts.count = 6;
      opts.offset = 0;
      opts.sort_group = 0;

      const size_t left = cache.size()-(_batch_sz*i);
      if (left < _batch_sz) {
        opts.instances = static_cast<uint32>(left);
      } else {
        opts.instances = static_cast<uint32>(_batch_sz);
      }

      ctx.submit_command({
        .target = fbo.handle(),
        .pipeline = _pipeline.handle(),
        .buffers = {&buf_binds[0], std::size(buf_binds)},
        .textures = {&tex_binds[0], std::size(tex_binds)},
        .uniforms = {&pconsts[0], std::size(pconsts)},
        .draw_opts = opts,
        .on_render = [this, &cache, i, count=opts.instances](auto) {
          const size_t offset = _batch_sz*i;
          // SHOGLE_LOG(debug, "Writing {} things from offset {}", count, offset);
          _ssbo.upload(0, count*sizeof(font_shader_glyph), cache.data()+offset);
          // std::memcpy(_ssbo_mapped, cache.data()+offset, count*sizeof(font_shader_glyph));
          // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        },
      });
    }
  }

private:
public:
  renderer_pipeline _pipeline;
  renderer_buffer _ssbo;
  renderer_texture _atlas_tex;
  font_atlas_data<char> _font;
  void* _ssbo_mapped;
  size_t _batch_sz;
};

} // namespace ntf
