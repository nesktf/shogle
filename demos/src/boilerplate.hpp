#pragma once

#include <shogle/render/texture.hpp>
#include <shogle/render/window.hpp>
#include <shogle/render/vertex.hpp>
#include <shogle/render/pipeline.hpp>
#include <shogle/render/framebuffer.hpp>

#include <shogle/assets/texture.hpp>

#include <shogle/scene/font.hpp>

#include <ntfstl/utility.hpp>

namespace shogle {

const inline external_state r_def_ext_state {
  .primitive = primitive_mode::triangles,
  .poly_mode = polygon_mode::fill,
  .poly_width = 1.f,
  .test = {
    .stencil_test = nullptr,
    .depth_test = nullptr,
    .scissor_test = nullptr,
    .face_culling = nullptr,
    .blending = nullptr,
  },
};

const inline blend_opts def_blending_opts {
  .mode = blend_mode::add,
  .src_factor = blend_factor::src_alpha,
  .dst_factor = blend_factor::inv_src_alpha,
  .color = {0.f, 0.f, 0.f, 0.f},
};

const inline depth_test_opts def_depth_opts {
  .func = test_func::less,
  .near_bound = 0.f,
  .far_bound = 1.f,
};

inline render_expect<texture2d> make_texture2d(context_view ctx, const bitmap_data& image,
                                        texture_sampler sampler,
                                        texture_addressing addressing,
                                        u32 levels = 7u, bool mips = true) {
  const auto desc = image.make_descriptor();
  const texture_data data {
    .images = {desc},
    .generate_mipmaps = mips && levels > 1,
  };
  return texture2d::create(ctx, {
    .format = image.format,
    .sampler = sampler,
    .addressing = addressing,
    .extent = image.extent,
    .layers = 1,
    .levels = levels,
    .data = &data,
  });
}

inline render_expect<texture2d> make_texture2d(context_view ctx, image_format format, extent2d extent,
                                        texture_sampler sampler,
                                        texture_addressing addressing,
                                        u32 levels = 7u) {
  return texture2d::create(ctx, {
    .format = format,
    .sampler = sampler,
    .addressing = addressing,
    .extent = meta::image_dim_traits<extent2d>::extent_cast(extent),
    .layers = 1,
    .levels = levels,
    .data = nullptr,
  });
}

template<meta::image_depth_type DepthT = uint8>
inline render_expect<texture2d> load_texture2d(context_view ctx, const std::string& path,
                                        texture_sampler sampler,
                                        texture_addressing addressing,
                                        u32 levels = 7u, bool mips = true) {
  return load_image<DepthT>(path)
  .and_then([&](auto&& image) -> render_expect<texture2d> {
    return make_texture2d(ctx, image, sampler, addressing, levels, mips);
  });
}

template<typename Vert>
requires(meta::is_aos_vertex<Vert>)
render_expect<pipeline> make_pipeline(
  vertex_shader_view vert, fragment_shader_view frag,
  weak_ptr<const depth_test_opts> depth_test = def_depth_opts,
  weak_ptr<const blend_opts> blending = def_blending_opts
) {
  auto ctx = vert.context();
  const auto attribs = Vert::aos_binding();
  const shader_t stages[] = {vert.get(), frag.get()};
  return pipeline::create(ctx, {
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages,
    .primitive = primitive_mode::triangles,
    .poly_mode = polygon_mode::fill,
    .poly_width = 1.f,
    .tests = {
      .stencil_test = nullptr,
      .depth_test = depth_test,
      .scissor_test = nullptr,
      .face_culling = nullptr,
      .blending = blending,
    }
  });
}

template<typename Vert>
requires(meta::is_aos_vertex<Vert>)
render_expect<pipeline> make_pipeline(
  context_view ctx,
  std::string_view vert_src, std::string_view frag_src,
  weak_ptr<const depth_test_opts> depth_test = def_depth_opts,
  weak_ptr<const blend_opts> blending = def_blending_opts
) {
  auto vert = vertex_shader::create(ctx, {vert_src});
  if (!vert) {
    return ntf::unexpected{std::move(vert.error())};
  }
  auto frag = fragment_shader::create(ctx, {frag_src});
  if (!frag) {
    return ntf::unexpected{std::move(frag.error())};
  }
  const auto attribs = Vert::aos_binding();
  const shader_t stages[] = {vert->get(), frag->get()};
  return pipeline::create(ctx, {
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages,
    .primitive = primitive_mode::triangles,
    .poly_mode = polygon_mode::fill,
    .poly_width = 1.f,
    .tests = {
      .stencil_test = nullptr,
      .depth_test = depth_test,
      .scissor_test = nullptr,
      .face_culling = nullptr,
      .blending = blending,
    }
  });
}

inline render_expect<std::pair<framebuffer, texture2d>> make_fbo(
  context_view ctx, const extent2d& extent,
  const color4& clear_color, clear_flag clear_flags,
  texture_sampler sampler = texture_sampler::nearest,
  texture_addressing addressing = texture_addressing::clamp_edge,
  image_format format = image_format::rgb8u,
  fbo_buffer test_buffer = fbo_buffer::depth24u_stencil8u
) {
  return texture2d::create(ctx, {
    .format = format,
    .sampler = sampler,
    .addressing = addressing,
    .extent = meta::image_dim_traits<extent2d>::extent_cast(extent),
    .layers = 1,
    .levels = 1,
    .data = nullptr,
  })
  .and_then([&](auto&& tex) -> render_expect<std::pair<framebuffer, texture2d>> {
    const fbo_image fb_img {
      .texture = tex.get(),
      .layer = 0u,
      .level = 0u,
    };
    auto fbo = framebuffer::create(ctx, {
      .extent = extent,
      .viewport = {0u, 0u, extent.x, extent.y},
      .clear_color = clear_color,
      .clear_flags = clear_flags,
      .test_buffer = test_buffer,
      .images = {fb_img},
    });
    if (!fbo) {
      return ntf::unexpected{std::move(fbo.error())};
    }
    return std::make_pair(std::move(*fbo), std::move(tex));
  });
}

inline render_expect<std::pair<window, context>> make_gl_ctx(
  u32 win_width, u32 win_height, const char* win_title,
  u32 swap_interval = 0,
  const color4& fb_color = {.3f, .3f, .3f, 1.f},
  clear_flag fb_clear = clear_flag::color_depth
) {
  const win_gl_params win_gl {
    .ver_major = 4,
    .ver_minor = 6,
    .swap_interval = swap_interval,
    .fb_msaa_level = 0,
    .fb_buffer = fbo_buffer::depth24u_stencil8u,
    .fb_use_alpha = false,
  };
  auto win = window::create({
    .width = win_width,
    .height = win_height,
    .title = win_title,
    .attrib = win_attrib::decorate | win_attrib::resizable,
    .renderer_api = context_api::opengl,
    .platform_params = nullptr,
    .renderer_params = &win_gl,
  });
  if (!win) {
    return {ntf::unexpect, render_error::unknown_error, win.error().msg()};
  }
  const auto vp = uvec4{0, 0, win->fb_size()};
  const auto gl_params = window::make_gl_params(*win);
  auto ctx = context::create({
    .ctx_params = &gl_params,
    .ctx_api = win->renderer(),
    .fb_viewport = vp,
    .fb_clear_flags = fb_clear,
    .fb_clear_color = fb_color,
    .alloc = nullptr,
  });
  if (!ctx) {
    return ntf::unexpected{std::move(ctx.error())};
  }
  return {ntf::in_place, std::move(*win), std::move(*ctx)};
}

inline render_expect<context> make_gl_ctx(
  const window& win,
  const color4& fb_color = {.3f, .3f, .3f, 1.f},
  clear_flag fb_clear = clear_flag::color_depth
) {
  if (win.renderer() != context_api::opengl) {
    return {ntf::unexpect, render_error::invalid_handle, "Invalid window context"};
  }
  const auto vp = uvec4{0, 0, win.fb_size()};
  const auto gl_params = window::make_gl_params(win);
  auto ctx = context::create({
    .ctx_params = &gl_params,
    .ctx_api = context_api::opengl,
    .fb_viewport = vp,
    .fb_clear_flags = fb_clear,
    .fb_clear_color = fb_color,
    .alloc = nullptr,
  });
  if (!ctx) {
    return ntf::unexpected{std::move(ctx.error())};
  }
  return std::move(*ctx);
}

template<font_codepoint_type CodeT = char>
render_expect<font_renderer> r_load_font(
  context_view ctx,
  const std::string& path,
  font_charset_view<CodeT> charset = ascii_charset<CodeT>,
  u32 glyph_size = 48u,
  u32 padding = 0u,
  font_load_flags flags = font_load_flags::render_sdf,
  texture_sampler sampler = texture_sampler::linear,
  uint64 batch_size = 64u
) {
  return load_font_atlas(path, flags, glyph_size, padding, charset)
  .and_then([&](auto&& atlas) -> render_expect<font_renderer> {
    return font_renderer::create(ctx, std::move(atlas), sampler, batch_size);
  });
}

} // namespace shogle
