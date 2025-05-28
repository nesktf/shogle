#include "./font.hpp"
#include "../assets/shaders/instanced_font.hpp"
#include "../render/shader.hpp"

namespace ntf {

namespace {

auto make_pipeline(r_shader_view vert, r_shader_view frag) {
  auto ctx = vert.context();
  const auto attribs = quad_mesh::attribute_binding();
  const r_shader stages[] = {vert.handle(), frag.handle()};
  const r_blend_opts blending_opts {
    .mode = r_blend_mode::add,
    .src_factor = ntf::r_blend_factor::src_alpha,
    .dst_factor = ntf::r_blend_factor::inv_src_alpha,
    .color = {0.f, 0.f, 0.f, 0.f},
  };
  const r_depth_test_opts depth_opts {
    .test_func = ntf::r_test_func::lequal,
    .near_bound = 0.f,
    .far_bound = 1.f,
  };

  return renderer_pipeline::create(ctx, {
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages,
    .primitive = ntf::r_primitive::triangles,
    .poly_mode = ntf::r_polygon_mode::fill,
    .poly_width = ntf::nullopt,
    .stencil_test = nullptr,
    .depth_test = depth_opts,
    .scissor_test = nullptr,
    .face_culling = nullptr,
    .blending = blending_opts,
  });
}

auto make_uniform_buffer(r_context_view ctx, size_t size) {
  return renderer_buffer::create(ctx, {
    .type = r_buffer_type::uniform,
    .flags = r_buffer_flag::dynamic_storage,
    .size = size,
    .data = nullptr,
  });
}

} // namespace

void text_buffer::_append_glyph(const glyph_metrics& glyph,
                                float x, float y, float scale,
                                float atlas_w, float atlas_h)
{
  const bool flip_y = true;
  const bool flip_x = false;

  const float gsize_x = static_cast<float>(glyph.size.x);
  const float gsize_y = static_cast<float>(glyph.size.y);

  const vec2 transf_scale {
    gsize_x*scale,
    gsize_y*scale,
  };
  const vec2 transf_offset {
    x + (gsize_x*.5f*scale + static_cast<float>(glyph.bearing.x)*scale),
    y - (gsize_y*.5f*scale - static_cast<float>(glyph.bearing.y)*scale)
  };

  const vec2 uv_scale {
    gsize_x/((flip_x*-1.f + !flip_x*1.f)*atlas_w),
    gsize_y/((flip_y*-1.f + !flip_y*1.f)*atlas_h)
  };
  const vec2 uv_offset {
    (static_cast<float>(glyph.offset.x) + (flip_x*gsize_x))/atlas_w,
    (static_cast<float>(glyph.offset.y) + (flip_y*gsize_y))/atlas_h
  };

  _glyph_cache.emplace_back(transf_scale, transf_offset, uv_scale, uv_offset);
}


sdf_text_rule::sdf_text_rule(renderer_pipeline&& pipeline, renderer_buffer&& uniform_buffer,
                             const glyph_props& props) :
  _pipeline{std::move(pipeline)}, _uniform_buffer{std::move(uniform_buffer)},
  _props{props} {}

r_expected<sdf_text_rule> sdf_text_rule::create(r_context_view ctx,
                                                const color3& color, float width, float edge,
                                                const color3& outline_color,
                                                const vec2& outline_offset,
                                                float outline_width, float outline_edge)
{
  auto frag = renderer_shader::create(ctx, {
    .type = r_shader_type::fragment,
    .source = {shad_frag_font_sdf},
  });
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }

  auto vert = renderer_shader::create(ctx, {
    .type = r_shader_type::vertex,
    .source = {shad_vert_instanced_font},
  });
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, *frag);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }

  auto buffer = make_uniform_buffer(ctx, sizeof(glyph_props));
  if (!buffer){
    return unexpected{std::move(buffer.error())};
  }

  glyph_props props;
  props.text_color = color;
  props.text_width = width;
  props.text_edge = edge;
  props.out_color = outline_color;
  props.out_offset = outline_offset;
  props.out_width = outline_width;
  props.out_edge = outline_edge;

  return sdf_text_rule{std::move(*pipeline), std::move(*buffer), props};
}

std::pair<r_pipeline, r_shader_buffer> sdf_text_rule::write_uniforms() {
  _uniform_buffer.upload(0u, sizeof(glyph_props), &_props);
  return std::make_pair(_pipeline.handle(), r_shader_buffer{
    .buffer = _uniform_buffer.handle(),
    .binding = 2,
    .offset = 0u,
    .size = sizeof(glyph_props),
  });
}


bitmap_text_rule::bitmap_text_rule(renderer_pipeline&& pipeline, renderer_buffer&& uniform_buffer,
                                   const color3& color) :
  _pipeline{std::move(pipeline)}, _uniform_buffer{std::move(uniform_buffer)},
  _text_color{color} {}

r_expected<bitmap_text_rule> bitmap_text_rule::create(r_context_view ctx, const color3& color) {
  auto frag = renderer_shader::create(ctx, {
    .type = r_shader_type::fragment,
    .source = {shad_frag_font_normal},
  });
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }

  auto vert = renderer_shader::create(ctx, {
    .type = r_shader_type::vertex,
    .source = {shad_vert_instanced_font},
  });
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, *frag);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }

  auto buffer = make_uniform_buffer(ctx, sizeof(color3));
  if (!buffer){
    return unexpected{std::move(buffer.error())};
  }

  return bitmap_text_rule{std::move(*pipeline), std::move(*buffer), color};
}

std::pair<r_pipeline, r_shader_buffer> bitmap_text_rule::write_uniforms() {
  _uniform_buffer.upload(0u, sizeof(color3), &_text_color);
  return std::make_pair(_pipeline.handle(), r_shader_buffer{
    .buffer = _uniform_buffer.handle(),
    .binding = 2,
    .offset = 0u,
    .size = sizeof(color3),
  });
}

font_renderer::font_renderer(renderer_buffer&& ssbo, renderer_texture&& atlas,
                             font_glyphs&& glyphs, glyph_map&& map,
                             const mat4& transform, vec2 bitmap_extent, size_t batch) noexcept :
  _ssbo{std::move(ssbo)}, _atlas_tex{std::move(atlas)},
  _glyphs{std::move(glyphs)}, _glyph_map{std::move(map)},
  _transform{transform}, _bitmap_extent{bitmap_extent}, _batch_sz{batch} {}

r_expected<font_renderer> font_renderer::create(r_context_view ctx,
                                                const mat4& transform,
                                                font_atlas_data&& font,
                                                r_texture_sampler sampler, size_t batch_size)
{
  const size_t ssbo_sz = batch_size * sizeof(text_buffer::glyph_entry);

  auto ssbo = renderer_buffer::create(ctx, {
    .type = r_buffer_type::shader_storage,
    .flags = r_buffer_flag::dynamic_storage,
    .size = ssbo_sz,
    .data = nullptr,
  });
  if (!ssbo) {
    return unexpected{std::move(ssbo.error())};
  }

  auto font_desc = font.make_bitmap_descriptor();
  auto tex = renderer_texture::create(ctx, {
    .type = r_texture_type::texture2d,
    .format = font.bitmap_format,
    .extent = tex_extent_cast(font.bitmap_extent),
    .layers = 1,
    .levels = 1,
    .images = {font_desc},
    .gen_mipmaps = false,
    .sampler = sampler,
    .addressing = r_texture_address::repeat,
  });
  if (!tex) {
    return unexpected{std::move(tex.error())};
  }

  return font_renderer{
    std::move(*ssbo), std::move(*tex),
    std::move(font.glyphs), std::move(font.map),
    transform, static_cast<vec2>(font.bitmap_extent), batch_size
  };
}

void font_renderer::ssbo_callback_t::operator()(r_context) const {
  // TODO: Investigate how to abstract GPU synchronization when writting to
  //       mapped buffers, to avoid rebinding for uploads each frame
  ssbo.upload(0u, glyph_count*sizeof(text_buffer::glyph_entry), buffer_data.data()+offset);
}

void font_renderer::clear_state() {
  _write_callbacks.clear();
}

void font_renderer::append_text(const text_buffer& buffer) {
  const auto buffer_data = buffer.data();
  const size_t batches = (buffer_data.size()/_batch_sz)+1;
  for (size_t i = 0; i < batches; ++i) {
    const size_t left = buffer_data.size()-(_batch_sz*i);
    size_t glyph_count;
    if (left < _batch_sz) {
      glyph_count = static_cast<uint32>(left);
    } else {
      glyph_count = static_cast<uint32>(_batch_sz);
    }
    _write_callbacks.emplace_back(_ssbo, buffer_data, glyph_count, _batch_sz*i);
  }
}

void font_renderer::render(const quad_mesh& quad, r_framebuffer_view fbo,
                           font_render_rule& render_rule, uint32 sort_group)
{
  auto ctx = fbo.context();
  auto [pipeline, unif_buffer] = render_rule.write_uniforms();

  const r_shader_buffer uniform_binds[] = {
    { .buffer = _ssbo.handle(), .binding = 1, .offset = 0u, .size = _ssbo.size() },
    unif_buffer,
  };
  auto atlas_tex_handle = _atlas_tex.handle();

  const r_push_constant unif_consts[] = {
    r_format_pushconst(r_pipeline_get_uniform(pipeline, "u_atlas_sampler").value(), 0),
    r_format_pushconst(r_pipeline_get_uniform(pipeline, "u_text_transform").value(), _transform),
  };
  for (auto& cb : _write_callbacks) {
    ctx.submit_command({
      .target = fbo.handle(),
      .pipeline = pipeline,
      .buffers = quad.bindings(uniform_binds),
      .textures = {atlas_tex_handle},
      .uniforms = unif_consts,
      .draw_opts = {
        .count = 6,
        .offset = 0,
        .instances = static_cast<uint32>(cb.glyph_count),
      },
      .sort_group = sort_group,
      .on_render = cb,
    });
  }
}
void font_renderer::render(const quad_mesh& quad, r_framebuffer_view fbo,
                           font_render_rule& render_rule, const text_buffer& buffer,
                           uint32 sort_group) {
  clear_state();
  append_text(buffer);
  render(quad, fbo, render_rule, sort_group);
}

} // namespace ntf
