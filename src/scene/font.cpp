#include "./font.hpp"
#include "../assets/shaders/instanced_font.hpp"
#include "../render/shader.hpp"

namespace ntf::render {

namespace {

auto make_pipeline(vertex_shader_view vert, fragment_shader_view frag) {
  auto ctx = vert.context();
  const auto attribs = quad_mesh::attribute_binding();
  const shader_t stages[] = {vert.get(), frag.get()};
  const blend_opts blending_opts {
    .mode = blend_mode::add,
    .src_factor = blend_factor::src_alpha,
    .dst_factor = blend_factor::inv_src_alpha,
    .color = {0.f, 0.f, 0.f, 0.f},
  };
  const depth_test_opts depth_opts {
    .func = test_func::lequal,
    .near_bound = 0.f,
    .far_bound = 1.f,
  };

  return pipeline::create(ctx, {
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages,
    .primitive = primitive_mode::triangles,
    .poly_mode = polygon_mode::fill,
    .poly_width = 1.f,
    .tests = {
      .stencil_test = nullptr,
      .depth_test = depth_opts,
      .scissor_test = nullptr,
      .face_culling = nullptr,
      .blending = blending_opts,
    },
  });
}

auto make_ubo(context_view ctx, size_t size) {
  return uniform_buffer::create(ctx, {
    .flags = buffer_flag::dynamic_storage,
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


sdf_text_rule::sdf_text_rule(pipeline&& pip, uniform_buffer&& ubo,
                             uniform_view u_sampler, uniform_view u_transf,
                             const glyph_props& props) :
  _pipeline{std::move(pip)}, _uniform_buffer{std::move(ubo)},
  _u_sampler{u_sampler}, _u_transf{u_transf},
  _props{props} {}

expect<sdf_text_rule> sdf_text_rule::create(context_view ctx,
                                            const color3& color, float width, float edge,
                                            const color3& outline_color,
                                            const vec2& outline_offset,
                                            float outline_width, float outline_edge)
{
  auto frag = fragment_shader::create(ctx, {shad_frag_font_sdf});
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }

  auto vert = vertex_shader::create(ctx, {shad_vert_instanced_font});
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, *frag);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  } 

  auto u_transf = pipeline->uniform("u_text_transform");
  auto u_sampler = pipeline->uniform("u_atlas_sampler");
  NTF_ASSERT(!u_transf.empty());
  NTF_ASSERT(!u_sampler.empty());

  auto buffer = make_ubo(ctx, sizeof(glyph_props));
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

  return sdf_text_rule{std::move(*pipeline), std::move(*buffer), u_sampler, u_transf, props};
}

font_render_data sdf_text_rule::write_uniforms() {
  _uniform_buffer.upload(_props);
  return {
    .pip = _pipeline,
    .binds = {
      .buffer = _uniform_buffer.get(),
      .binding = 2,
      .size = sizeof(glyph_props),
      .offset = 0u,
    },
    .u_sampler = _u_sampler,
    .u_transf = _u_transf,
  };
}


bitmap_text_rule::bitmap_text_rule(pipeline&& pip, uniform_buffer&& ubo,
                                   uniform_view u_sampler, uniform_view u_transf,
                                   const color3& color) :
  _pipeline{std::move(pip)}, _uniform_buffer{std::move(ubo)},
  _u_sampler{u_sampler}, _u_transf{u_transf},
  _text_color{color} {}

expect<bitmap_text_rule> bitmap_text_rule::create(context_view ctx, const color3& color) {
  auto frag = fragment_shader::create(ctx, {shad_frag_font_normal});
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }

  auto vert = vertex_shader::create(ctx, {shad_vert_instanced_font});
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }

  auto pipeline = make_pipeline(*vert, *frag);
  if (!pipeline) {
    return unexpected{std::move(pipeline.error())};
  }
  auto u_transf = pipeline->uniform("u_text_transform");
  auto u_sampler = pipeline->uniform("u_atlas_sampler");
  NTF_ASSERT(!u_transf.empty());
  NTF_ASSERT(!u_sampler.empty());

  auto buffer = make_ubo(ctx, sizeof(color3));
  if (!buffer){
    return unexpected{std::move(buffer.error())};
  }

  return bitmap_text_rule{std::move(*pipeline), std::move(*buffer), u_sampler, u_transf, color};
}

font_render_data bitmap_text_rule::write_uniforms() {
  _uniform_buffer.upload(_text_color);
  return {
    .pip = _pipeline,
    .binds = {
      .buffer = _uniform_buffer.get(),
      .binding = 2,
      .size = sizeof(color3),
      .offset = 0u,
    },
    .u_sampler = _u_sampler,
    .u_transf = _u_transf,
  };
}

font_renderer::font_renderer(shader_storage_buffer&& ssbo, texture2d&& atlas,
                             font_glyphs&& glyphs, glyph_map&& map,
                             const mat4& transform, vec2 bitmap_extent, size_t batch) noexcept :
  _ssbo{std::move(ssbo)}, _atlas_tex{std::move(atlas)},
  _glyphs{std::move(glyphs)}, _glyph_map{std::move(map)},
  _transform{transform}, _bitmap_extent{bitmap_extent}, _batch_sz{batch} {}

expect<font_renderer> font_renderer::create(context_view ctx,
                                            const mat4& transform,
                                            font_atlas_data&& font,
                                            texture_sampler sampler, size_t batch_size)
{
  const size_t ssbo_sz = batch_size * sizeof(text_buffer::glyph_entry);

  auto ssbo = shader_storage_buffer::create(ctx, {
    .flags = buffer_flag::dynamic_storage,
    .size = ssbo_sz,
    .data = nullptr,
  });
  if (!ssbo) {
    return unexpected{std::move(ssbo.error())};
  }

  auto font_desc = font.make_bitmap_descriptor();
  const texture_data data {
    .images = {font_desc},
    .generate_mipmaps = false,
  };
  auto tex = texture2d::create(ctx, {
    .format = font.bitmap_format,
    .sampler = sampler,
    .addressing = texture_addressing::repeat,
    .extent = image_extent_cast(font.bitmap_extent),
    .layers = 1,
    .levels = 1,
    .data = data,
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

void font_renderer::ssbo_callback_t::operator()(context_t) const {
  // TODO: Investigate how to abstract GPU synchronization when writting to
  //       mapped buffers, to avoid rebinding for uploads each frame
  ssbo.upload(glyph_count*sizeof(text_buffer::glyph_entry), 0u, buffer_data.data()+offset);
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

void font_renderer::render(const quad_mesh& quad, framebuffer_view fbo,
                           font_render_rule& render_rule, uint32 sort_group)
{
  auto ctx = fbo.context();
  auto data = render_rule.write_uniforms();

  const shader_binding uniform_binds[] = {
    {
      .buffer = _ssbo.get(),
      .binding = 1,
      .size = _ssbo.size(),
      .offset = 0u,
    },
    data.binds,
  };
  const int32 sampler = 0;
  const texture_binding tbind {.texture = _atlas_tex, .sampler = sampler};

  const uniform_const unif_consts[] = {
    format_uniform_const(data.u_sampler, sampler),
    format_uniform_const(data.u_transf, _transform),
  };
  for (auto& cb : _write_callbacks) {
    ctx.submit_render_command({
      .target = fbo.get(),
      .pipeline = data.pip,
      .buffers = quad.bindings(uniform_binds),
      .textures = {tbind},
      .consts = unif_consts,
      .opts = {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = static_cast<uint32>(cb.glyph_count),
      },
      .sort_group = sort_group,
      .render_callback = cb,
    });
  }
}

void font_renderer::render(const quad_mesh& quad, framebuffer_view fbo,
                           font_render_rule& render_rule, const text_buffer& buffer,
                           uint32 sort_group)
{
  clear_state();
  append_text(buffer);
  render(quad, fbo, render_rule, sort_group);
}

} // namespace ntf::render
