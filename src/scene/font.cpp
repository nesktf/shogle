#include "./font.hpp"

namespace ntf {

font_renderer::font_renderer(renderer_pipeline pip, renderer_buffer ssbo, renderer_texture atlas,
                             font_atlas_data&& font_data, size_t batch) :
  _pipeline{std::move(pip)}, _ssbo{std::move(ssbo)}, _atlas_tex{std::move(atlas)},
  _glyphs{std::move(font_data.glyphs)}, _glyph_map{std::move(font_data.glyph_map)},
  _bitmap_extent{font_data.bitmap_extent}, _batch_sz{batch} {}

auto font_renderer::create(
  r_context_view ctx,
  weak_ref<r_attrib_binding> attrib_bind, span_view<r_attrib_descriptor> attrib_desc,
  std::string_view vert_src, std::string_view frag_src,
  font_atlas_data&& font, size_t batch_size
) -> asset_expected<font_renderer> {
  const size_t ssbo_sz = batch_size * sizeof(font_shader_glyph);

  auto vert = renderer_shader::create(ctx, {
    .type = ntf::r_shader_type::vertex,
    .source = {vert_src},
  });
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto frag = renderer_shader::create(ctx, {
    .type = ntf::r_shader_type::fragment,
    .source = {frag_src},
  });
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }
  ntf::r_shader shads[] = {vert->handle(), frag->handle()};
  auto pipeline = renderer_pipeline::create(ctx, {
    .stages = {&shads[0], std::size(shads)},
    .attrib_binding = attrib_bind,
    .attrib_desc = attrib_desc,
    .primitive = ntf::r_primitive::triangles,
    .poly_mode = ntf::r_polygon_mode::fill,
    .front_face = ntf::r_front_face::clockwise,
    .cull_mode = ntf::r_cull_mode::front_back,
    .tests = ntf::r_pipeline_test::all,
    .depth_compare_op = ntf::r_compare_op::less,
    .stencil_compare_op = ntf::r_compare_op::less,
  });
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }

  auto ssbo = renderer_buffer::create(ctx, {
    .type = ntf::r_buffer_type::shader_storage,
    .flags = ntf::r_buffer_flag::dynamic_storage,
    .size = ssbo_sz,
    .data = nullptr,
  });
  if (!ssbo) {
    return unexpected{std::move(ssbo.error())};
  }

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
  if (!tex) {
    return unexpected{std::move(tex.error())};
  }

  return font_renderer{
    std::move(*pipeline), std::move(*ssbo), std::move(*tex), std::move(font), batch_size
  };

}

void font_renderer::render(const font_render_cache& cache, r_framebuffer_view fbo,
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
        // TODO: Investigate how to abstract GPU synchronization when writting to
        //       mapped buffers, to avoid rebinding for uploads each frame
        const size_t offset = _batch_sz*i;
        _ssbo.upload(0, count*sizeof(font_shader_glyph), cache.data()+offset);
      },
    });
  }
}

void font_renderer::_append_codepoint(char32_t code, font_render_cache& cache,
                                      float start_x, float start_y, float scale,
                                      float& x, float& y) {
  const bool flip_y = true;
  const bool flip_x = false;
  size_t idx = 0u;
  if (auto g_it = _glyph_map.find(code); g_it != _glyph_map.end()) {
    idx = static_cast<size_t>(g_it->second);
  }
  const auto& glyph = _glyphs[idx];
  if (code == '\n') {
    x = start_x;
    y -= static_cast<float>(glyph.advance.y)*scale;
    return;
  }

  const float tex_x = static_cast<float>(_bitmap_extent.x);
  const float tex_y = static_cast<float>(_bitmap_extent.y);
  const float gsize_x = static_cast<float>(glyph.size.x);
  const float gsize_y = static_cast<float>(glyph.size.y);

  const vec2 transf_scale {
    gsize_x*scale,
    gsize_y*scale,
  };
  const vec2 transf_offset {
    x + (gsize_x*.5f + static_cast<float>(glyph.bearing.x)*scale),
    y - (gsize_y*.5f - static_cast<float>(glyph.bearing.y)*scale)
  };

  const vec2 uv_scale {
    gsize_x/((flip_x*-1.f + !flip_x*1.f)*tex_x),
    gsize_y/((flip_y*-1.f + !flip_y*1.f)*tex_y)
  };
  const vec2 uv_offset {
    (static_cast<float>(glyph.offset.x) + (flip_x*gsize_x))/tex_x,
    (static_cast<float>(glyph.offset.y) + (flip_y*gsize_y))/tex_y
  };

  cache.emplace_back(transf_scale, transf_offset, uv_scale, uv_offset);
  
  x += static_cast<float>(glyph.advance.x)*scale;
}

} // namespace ntf
