#include "./font.hpp"
#include "../assets/shaders/instanced_font.hpp"

// TODO: Validate uniform type!
#define RET_IF_NO_UNIFORM(_name, _type, _pipeline) \
auto _name = _pipeline.uniform(#_name); \
if (!_name) { \
  return unexpected{r_error::format({"Uniform not found '{}'"}, #_name)}; \
}

namespace ntf {

namespace {

auto make_pipeline(r_shader_view vert, r_shader_view frag) {
  auto ctx = vert.context();
  auto attrib_bind = quad_mesh::attr_binding();
  auto attrib_desc = quad_mesh::attr_descriptor();
  r_shader shads[] = {vert.handle(), frag.handle()};
  return renderer_pipeline::create(ctx, {
    .stages = {&shads[0], std::size(shads)},
    .attrib_binding = attrib_bind,
    .attrib_desc = attrib_desc,
    .primitive = r_primitive::triangles,
    .poly_mode = r_polygon_mode::fill,
    .front_face = r_front_face::clockwise,
    .cull_mode = r_cull_mode::front_back,
    .tests = r_pipeline_test::depth,
    .depth_compare_op = r_compare_op::lequal,
    .stencil_compare_op = nullopt,
  });
}

void upload_commands(const quad_mesh& quad, r_buffer_view ssbo,
                     r_framebuffer fbo, r_pipeline pipeline,
                     r_texture atlas_tex, uint32 sampler,
                     size_t batch_sz, size_t batches,
                     span_view<font_shader_glyph> cache,
                     span_view<r_push_constant> pconsts)
{
  auto ctx = ssbo.context();

  const r_buffer_binding buf_binds[] = {
    {quad.vbo().handle(), r_buffer_type::vertex, nullopt},
    {quad.ebo().handle(), r_buffer_type::index, nullopt},
    {ssbo.handle(), r_buffer_type::shader_storage, 1},
  };
  const r_texture_binding tex_binds[] = {
    {.texture = atlas_tex, .location = sampler},
  };

  for (size_t i = 0; i < batches; ++i) {
    r_draw_opts opts;
    opts.count = 6;
    opts.offset = 0;
    opts.sort_group = 0;

    const size_t left = cache.size()-(batch_sz*i);
    if (left < batch_sz) {
      opts.instances = static_cast<uint32>(left);
    } else {
      opts.instances = static_cast<uint32>(batch_sz);
    }

    ctx.submit_command({
      .target = fbo,
      .pipeline = pipeline,
      .buffers = {&buf_binds[0], std::size(buf_binds)},
      .textures = {&tex_binds[0], std::size(tex_binds)},
      .uniforms = pconsts,
      .draw_opts = opts,
      .on_render = [ssbo, cache, i, offset=batch_sz*i, count=opts.instances](auto) {
        // TODO: Investigate how to abstract GPU synchronization when writting to
        //       mapped buffers, to avoid rebinding for uploads each frame
        ssbo.upload(0u, count*sizeof(font_shader_glyph), cache.data()+offset);
      },
    });
  }
}

} // namespace

text_buffer_base::text_buffer_base(renderer_pipeline pipeline) noexcept :
  _pipeline{std::move(pipeline)} {}

void text_buffer_base::append(float x, float y, float scale, float atlas_w, float atlas_h,
                              const glyph_metrics& glyph)
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
    x + (gsize_x*.5f + static_cast<float>(glyph.bearing.x)*scale),
    y - (gsize_y*.5f - static_cast<float>(glyph.bearing.y)*scale)
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

void text_buffer_base::clear() { _glyph_cache.clear(); }

text_buffer::text_buffer(renderer_pipeline pipeline, const color3& color,
                         r_uniform u_transf, r_uniform u_sampler, r_uniform u_color) noexcept :
  text_buffer_base{std::move(pipeline)},
  _text_color{color}, _u_transf{u_transf}, _u_sampler{u_sampler}, _u_color{u_color} {}

r_expected<text_buffer> text_buffer::create(r_shader_view fragment, const color3& color) {
  auto ctx = fragment.context();

  auto vert = renderer_shader::create(ctx, {
    .type = r_shader_type::vertex,
    .source = {shad_vert_instanced_font},
  });
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, fragment);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }

  RET_IF_NO_UNIFORM(u_text_transform, r_attrib_type::mat4, (*pipeline));
  RET_IF_NO_UNIFORM(u_atlas_sampler, r_attrib_type::i32, (*pipeline));

  RET_IF_NO_UNIFORM(u_text_color, r_attrib_type::vec3, (*pipeline));

  return text_buffer{
    std::move(*pipeline), color,
    *u_text_transform, *u_atlas_sampler, *u_text_color
  };
}

r_expected<text_buffer> text_buffer::create(r_context_view ctx, const color3& color) {
  auto frag = renderer_shader::create(ctx, {
    .type = r_shader_type::fragment,
    .source = {shad_frag_font_normal},
  });
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }

  return create(*frag, color);
}

auto text_buffer::uniforms(const int32& sampler, const mat4& transf) const -> uniform_array {
  return {{
    r_format_pushconst(_u_transf, transf),
    r_format_pushconst(_u_sampler, sampler),
    r_format_pushconst(_u_color, _text_color),
  }};
}

sdf_text_buffer::sdf_text_buffer(renderer_pipeline pipeline, text_props props,
                                 r_uniform u_transf, r_uniform u_sampler,
                                 r_uniform u_color, r_uniform u_width, r_uniform u_edge,
                                 r_uniform u_out_color, r_uniform u_out_off,
                                 r_uniform u_out_width, r_uniform u_out_edge) noexcept :
  text_buffer_base{std::move(pipeline)},
  _props{std::move(props)},
  _u_transf{u_transf}, _u_sampler{u_sampler},
  _u_color{u_color}, _u_width{u_width}, _u_edge{u_edge},
  _u_out_color{u_out_color}, _u_out_offset{u_out_off},
  _u_out_width{u_out_width}, _u_out_edge{u_out_edge} {}

r_expected<sdf_text_buffer> sdf_text_buffer::create(r_shader_view fragment,
                                                    const color3& color, float width, float edge)
{
  auto ctx = fragment.context();

  auto vert = renderer_shader::create(ctx, {
    .type = r_shader_type::vertex,
    .source = {shad_vert_instanced_font},
  });
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, fragment);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }

  text_props props{};
  props.text_color = color;
  props.text_width = width;
  props.text_edge = edge;

  return _make_buffer(std::move(*pipeline), std::move(props));
}

r_expected<sdf_text_buffer> sdf_text_buffer::create(r_shader_view fragment,
                                                    const color3& color, float width, float edge,
                                                    const color3& outline_color,
                                                    const vec2& outline_offset,
                                                    float outline_width, float outline_edge)
{
  auto ctx = fragment.context();

  auto vert = renderer_shader::create(ctx, {
    .type = r_shader_type::vertex,
    .source = {shad_vert_instanced_font},
  });
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, fragment);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }

  text_props props;
  props.text_color = color;
  props.text_width = width;
  props.text_edge = edge;
  props.out_color = outline_color;
  props.out_offset = outline_offset;
  props.out_width = outline_width;
  props.out_edge = outline_edge;

  return _make_buffer(std::move(*pipeline), std::move(props));
}

r_expected<sdf_text_buffer> sdf_text_buffer::create(r_context_view ctx,
                                                    const color3& color, float width, float edge) {
  auto frag = renderer_shader::create(ctx, {
    .type = r_shader_type::fragment,
    .source = {shad_frag_font_sdf},
  });
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }

  return create(*frag, color, width, edge);
}

r_expected<sdf_text_buffer> sdf_text_buffer::create(r_context_view ctx,
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

  return create(*frag, color, width, edge,
                outline_color, outline_offset, outline_width, outline_edge);
}

r_expected<sdf_text_buffer> sdf_text_buffer::_make_buffer(renderer_pipeline pipeline,
                                                          text_props props)
{
  RET_IF_NO_UNIFORM(u_text_transform, r_attrib_type::mat4, pipeline);
  RET_IF_NO_UNIFORM(u_atlas_sampler, r_attrib_type::i32, pipeline);

  RET_IF_NO_UNIFORM(u_text_color, r_attrib_type::vec3, pipeline);
  RET_IF_NO_UNIFORM(u_text_width, r_attrib_type::f32, pipeline);
  RET_IF_NO_UNIFORM(u_text_edge, r_attrib_type::f32, pipeline);

  RET_IF_NO_UNIFORM(u_text_out_color, r_attrib_type::vec3, pipeline);
  RET_IF_NO_UNIFORM(u_text_out_offset, r_attrib_type::vec2, pipeline);
  RET_IF_NO_UNIFORM(u_text_out_width, r_attrib_type::f32, pipeline);
  RET_IF_NO_UNIFORM(u_text_out_edge, r_attrib_type::f32, pipeline);

  return sdf_text_buffer{
    std::move(pipeline), props,
    *u_text_transform, *u_atlas_sampler,
    *u_text_color, *u_text_width, *u_text_edge,
    *u_text_out_color, *u_text_out_offset, *u_text_out_width, *u_text_out_edge
  };
}

auto sdf_text_buffer::uniforms(const int32& sampler, const mat4& transf) const -> uniform_array {
  return {{
    r_format_pushconst(_u_transf, transf),
    r_format_pushconst(_u_sampler, sampler),
    r_format_pushconst(_u_color, _props.text_color),
    r_format_pushconst(_u_width, _props.text_width),
    r_format_pushconst(_u_edge, _props.text_edge),
    r_format_pushconst(_u_out_color, _props.out_color),
    r_format_pushconst(_u_out_offset, _props.out_offset),
    r_format_pushconst(_u_out_width, _props.out_width),
    r_format_pushconst(_u_out_edge, _props.out_edge),
  }};
}

font_renderer::font_renderer(renderer_buffer ssbo, renderer_texture atlas,
                             font_atlas_data::glyphs_t&& glyphs,
                             font_atlas_data::map_t&& glyph_map,
                             vec2 bitmap_extent, size_t batch) noexcept :
  _ssbo{std::move(ssbo)}, _atlas_tex{std::move(atlas)},
  _glyphs{std::move(glyphs)}, _glyph_map{std::move(glyph_map)},
  _bitmap_extent{bitmap_extent}, _batch_sz{batch} {}

r_expected<font_renderer> font_renderer::create(r_context_view ctx, font_atlas_data&& font,
                                                r_texture_sampler sampler, size_t batch_size) {
  const size_t ssbo_sz = batch_size * sizeof(font_shader_glyph);

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
    std::move(font.glyphs), std::move(font.glyph_map),
    static_cast<vec2>(font.bitmap_extent), batch_size
  };
}

void font_renderer::render(const text_buffer& buffer, r_framebuffer_view fbo,
                           const quad_mesh& quad, const mat4& transf_mat,
                           span_view<r_push_constant> pconsts)
{
  const int32 sampler = 0u;
  auto buffer_pconsts = buffer.uniforms(sampler, transf_mat);
  const bool use_cache = pconsts.size() > 0u;
  if (use_cache) {
    _pconst_cache.clear();
    for (const auto& unif : buffer_pconsts) {
      _pconst_cache.emplace_back(unif);
    }
    for (const auto& unif : pconsts) {
      _pconst_cache.emplace_back(unif);
    }
  }

  auto cache = buffer.cache();
  const size_t batches = (cache.size()/_batch_sz)+1;
  upload_commands(quad, _ssbo,
                  fbo.handle(), buffer.pipeline().handle(),
                  _atlas_tex.handle(), sampler,
                  _batch_sz, batches, cache,
                  use_cache ? span_view{_pconst_cache.data(), _pconst_cache.size()} :
                              span_view{buffer_pconsts.data(), buffer_pconsts.size()});
}

void font_renderer::render(const sdf_text_buffer& buffer, r_framebuffer_view fbo,
                           const quad_mesh& quad, const mat4& transf_mat,
                           span_view<r_push_constant> pconsts)
{
  const int32 sampler = 0u;
  auto buffer_pconsts = buffer.uniforms(sampler, transf_mat);
  const bool use_cache = pconsts.size() > 0u;
  if (use_cache) {
    _pconst_cache.clear();
    for (const auto& unif : buffer_pconsts) {
      _pconst_cache.emplace_back(unif);
    }
    for (const auto& unif : pconsts) {
      _pconst_cache.emplace_back(unif);
    }
  }

  auto cache = buffer.cache();
  const size_t batches = (cache.size()/_batch_sz)+1;
  upload_commands(quad, _ssbo,
                  fbo.handle(), buffer.pipeline().handle(),
                  _atlas_tex.handle(), sampler,
                  _batch_sz, batches, cache,
                  use_cache ? span_view{_pconst_cache.data(), _pconst_cache.size()} :
                              span_view{buffer_pconsts.data(), buffer_pconsts.size()});
}

void font_renderer::_append_codepoint(char32_t code, text_buffer_base& buffer,
                                      float start_x, float start_y, float scale,
                                      float& x, float& y) {
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

  buffer.append(x, y, scale, _bitmap_extent.x, _bitmap_extent.y, glyph);
  
  x += static_cast<float>(glyph.advance.x)*scale;
}

} // namespace ntf
