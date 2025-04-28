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
  const auto attribs = quad_mesh::attr_descriptor();
  const r_shader stages[] = {vert.handle(), frag.handle()};
  const r_blend_opts blending_opts {
    .mode = r_blend_mode::add,
    .src_factor = ntf::r_blend_factor::src_alpha,
    .dst_factor = ntf::r_blend_factor::inv_src_alpha,
    .color = {0.f, 0.f, 0.f, 0.f},
    .dynamic = false,
  };
  const r_depth_test_opts depth_opts {
    .test_func = ntf::r_test_func::less,
    .near_bound = 0.f,
    .far_bound = 1.f,
    .dynamic = false,
  };

  return renderer_pipeline::create(ctx, {
    .attrib_binding = 0u,
    .attrib_stride = quad_mesh::attr_stride(),
    .attribs = attribs,
    .stages = stages,
    .primitive = ntf::r_primitive::triangles,
    .poly_mode = ntf::r_polygon_mode::fill,
    .stencil_test = nullptr,
    .depth_test = depth_opts,
    .scissor_test = nullptr,
    .face_culling = nullptr,
    .blending = blending_opts,
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


sdf_text_rule::sdf_text_rule(renderer_pipeline pipeline, const text_props& props,
                             const mat4& transform, const uniform_handles& uniforms) :
  _pipeline{std::move(pipeline)}, _unifs{uniforms},
  _transform{transform}, _props{props}, _sampler{font_renderer::ATLAS_SAMPLER} {}

r_expected<sdf_text_rule> sdf_text_rule::create(r_context_view ctx, const mat4& transform,
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

  text_props props;
  props.text_color = color;
  props.text_width = width;
  props.text_edge = edge;
  props.out_color = outline_color;
  props.out_offset = outline_offset;
  props.out_width = outline_width;
  props.out_edge = outline_edge;

  RET_IF_NO_UNIFORM(u_text_transform, r_attrib_type::mat4, (*pipeline));
  RET_IF_NO_UNIFORM(u_atlas_sampler, r_attrib_type::i32, (*pipeline));

  RET_IF_NO_UNIFORM(u_text_color, r_attrib_type::vec3, (*pipeline));
  RET_IF_NO_UNIFORM(u_text_width, r_attrib_type::f32, (*pipeline));
  RET_IF_NO_UNIFORM(u_text_edge, r_attrib_type::f32, (*pipeline));

  RET_IF_NO_UNIFORM(u_text_out_color, r_attrib_type::vec3, (*pipeline));
  RET_IF_NO_UNIFORM(u_text_out_offset, r_attrib_type::vec2, (*pipeline));
  RET_IF_NO_UNIFORM(u_text_out_width, r_attrib_type::f32, (*pipeline));
  RET_IF_NO_UNIFORM(u_text_out_edge, r_attrib_type::f32, (*pipeline));

  uniform_handles unifs;
  unifs[U_TRANSFORM] = *u_text_transform;
  unifs[U_SAMPLER] = *u_atlas_sampler;
  unifs[U_COLOR] = *u_text_color;
  unifs[U_WIDTH] = *u_text_width;
  unifs[U_EDGE] = *u_text_edge;
  unifs[U_OUT_COLOR] = *u_text_out_color;
  unifs[U_OUT_OFFSET] = *u_text_out_offset;
  unifs[U_OUT_WIDTH] = *u_text_out_width;
  unifs[U_OUT_EDGE] = *u_text_out_edge;

  return sdf_text_rule{std::move(*pipeline), props, transform, unifs};
}

r_pipeline sdf_text_rule::retrieve_uniforms(uniform_list& uniforms) {
  uniforms.emplace_back(r_format_pushconst(_unifs[U_TRANSFORM], _transform));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_SAMPLER], _sampler));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_COLOR], _props.text_color));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_WIDTH], _props.text_width));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_EDGE], _props.text_edge));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_OUT_COLOR], _props.out_color));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_OUT_OFFSET], _props.out_offset));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_OUT_WIDTH], _props.out_width));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_OUT_EDGE], _props.out_edge));
  return _pipeline.handle();
}


bitmap_text_rule::bitmap_text_rule(renderer_pipeline pipeline, const color3& color,
                                   const mat4& transform, const uniform_handles& uniforms) :
  _pipeline{std::move(pipeline)}, _unifs{uniforms},
  _transform{transform}, _text_color{color}, _sampler{font_renderer::ATLAS_SAMPLER} {}

r_expected<bitmap_text_rule> bitmap_text_rule::create(r_context_view ctx, const mat4& transform,
                                                      const color3& color)
{
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

  RET_IF_NO_UNIFORM(u_text_transform, r_attrib_type::mat4, (*pipeline));
  RET_IF_NO_UNIFORM(u_atlas_sampler, r_attrib_type::i32, (*pipeline));

  RET_IF_NO_UNIFORM(u_text_color, r_attrib_type::vec3, (*pipeline));
  
  uniform_handles uniforms;
  uniforms[U_TRANSFORM] = *u_text_transform;
  uniforms[U_SAMPLER] = *u_atlas_sampler;
  uniforms[U_COLOR] = *u_text_color;

  return bitmap_text_rule{std::move(*pipeline), color, transform, uniforms};
}

r_pipeline bitmap_text_rule::retrieve_uniforms(uniform_list& uniforms) {
  uniforms.emplace_back(r_format_pushconst(_unifs[U_TRANSFORM], _transform));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_SAMPLER], _sampler));
  uniforms.emplace_back(r_format_pushconst(_unifs[U_COLOR], _text_color));
  return _pipeline.handle();
}

font_renderer::font_renderer(renderer_buffer ssbo, renderer_texture atlas,
                             font_glyphs&& glyphs, glyph_map&& map,
                             vec2 bitmap_extent, size_t batch) noexcept :
  _ssbo{std::move(ssbo)}, _atlas_tex{std::move(atlas)},
  _glyphs{std::move(glyphs)}, _glyph_map{std::move(map)},
  _bitmap_extent{bitmap_extent}, _batch_sz{batch} {}

r_expected<font_renderer> font_renderer::create(r_context_view ctx, font_atlas_data&& font,
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
    static_cast<vec2>(font.bitmap_extent), batch_size
  };
}

void font_renderer::ssbo_callback_t::operator()(r_context ctx) const {
  // TODO: Investigate how to abstract GPU synchronization when writting to
  //       mapped buffers, to avoid rebinding for uploads each frame
  ssbo.upload(0u, glyph_count*sizeof(text_buffer::glyph_entry), buffer_data.data()+offset);
}

void font_renderer::render(const quad_mesh& quad, r_framebuffer_view fbo,
                           rendering_rule& render_rule, const text_buffer& buffer,
                           uint32 sort_group)
{
  auto ctx = fbo.context();

  _write_callbacks.clear();
  _uniform_cache.clear();
  auto pipeline = render_rule.retrieve_uniforms(_uniform_cache);
  const auto buffer_data = buffer.data();
  const size_t batches = (buffer_data.size()/_batch_sz)+1;

  const r_buffer_binding buf_binds[] = {
    {quad.vbo().handle(), r_buffer_type::vertex, nullopt},
    {quad.ebo().handle(), r_buffer_type::index, nullopt},
    {_ssbo.handle(), r_buffer_type::shader_storage, 1},
  };
  const r_texture_binding tex_binds[] = {
    {.texture = _atlas_tex.handle(), .location = ATLAS_SAMPLER},
  };

  for (size_t i = 0; i < batches; ++i) {
    r_draw_opts opts;
    opts.count = 6;
    opts.offset = 0;
    opts.sort_group = sort_group;

    const size_t left = buffer_data.size()-(_batch_sz*i);
    if (left < _batch_sz) {
      opts.instances = static_cast<uint32>(left);
    } else {
      opts.instances = static_cast<uint32>(_batch_sz);
    }
    _write_callbacks.emplace_back(_ssbo, buffer_data, opts.instances, _batch_sz*i);

    ctx.submit_command({
      .target = fbo.handle(),
      .pipeline = pipeline,
      .buffers = buf_binds,
      .textures = tex_binds,
      .uniforms = {_uniform_cache.data(), _uniform_cache.size()},
      .draw_opts = opts,
      .on_render = _write_callbacks.back(),
    });
  }
}

} // namespace ntf
