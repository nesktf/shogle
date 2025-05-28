#pragma once

#include "./render.hpp"
#include "./assets.hpp"
#include "./scene.hpp"

namespace ntf {

template<typename F>
concept delta_time_func = std::invocable<F, double>; // f(dt) -> void

template<typename F>
concept fixed_render_func = std::invocable<F, double, double>; // f(dt, alpha) -> void

template<typename F>
concept fixed_update_func = std::invocable<F, uint32>; // f(ups) -> void

template<typename T>
concept has_render_member = requires(T t) {
  { t.on_render(double{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_render_member = requires(T t) {
  { t.on_render(double{}, double{}) } -> std::convertible_to<void>;
};

template<typename T>
concept has_fixed_update_member = requires(T t) {
  { t.on_fixed_update(uint32{}) } -> std::convertible_to<void>;
};

template<typename T>
concept fixed_loop_object = 
  (fixed_render_func<T> && fixed_update_func<T>) ||
  (has_fixed_render_member<T> && has_fixed_update_member<T>);

template<typename T>
concept nonfixed_loop_object = delta_time_func<T> || has_render_member<T>;


template<nonfixed_loop_object LoopObj>
void shogle_render_loop(renderer_window& window, r_context_view ctx, LoopObj&& obj) {
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_render_loop] Main loop started");

  time_point last_time = clock::now();
  while (!window.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
last_time = start_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};

    window.poll_events();

    ctx.start_frame();
    if constexpr (has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_render_loop] Main loop exit");
}

template<fixed_loop_object LoopObj>
void shogle_render_loop(renderer_window& window, r_context_view ctx, const uint32& ups,
                        LoopObj&& obj) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_render_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      if constexpr (has_fixed_update_member<LoopObj>) {
        obj.on_fixed_update(ups);
      } else {
        obj(ups);
      }
      lag -= fixed_elapsed_time;
    }

    if constexpr (has_fixed_render_member<LoopObj>) {
      obj.on_render(dt, alpha);
    } else {
      obj(dt, alpha);
    }

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_render_loop] Main loop exit");
}

template<fixed_render_func RFunc, fixed_update_func UFunc>
void shogle_render_loop(renderer_window& window, r_context_view ctx, const uint32& ups,
                        RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntf::shogle_render_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!window.should_close()) {
    fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    window.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      fixed_update(ups);
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntf::shogle_render_loop] Main loop exit");
}

const inline r_external_state r_def_ext_state {
  .primitive = r_primitive::triangles,
  .poly_mode = r_polygon_mode::fill,
  .poly_width = ntf::nullopt,
  .stencil_test = nullptr,
  .depth_test = nullptr,
  .scissor_test = nullptr,
  .face_culling = nullptr,
  .blending = nullptr,
};

const inline r_blend_opts r_def_blending_opts {
  .mode = r_blend_mode::add,
  .src_factor = r_blend_factor::src_alpha,
  .dst_factor = r_blend_factor::inv_src_alpha,
  .color = {0.f, 0.f, 0.f, 0.f},
};

const inline r_depth_test_opts r_def_depth_opts {
  .test_func = r_test_func::less,
  .near_bound = 0.f,
  .far_bound = 1.f,
};

inline r_expected<renderer_texture> r_make_texture2d(r_context_view ctx, const r_image_data& desc,
                                                     r_texture_sampler sampler,
                                                     r_texture_address addressing,
                                                     uint32 levels = 7u, bool mips = true) {
  return renderer_texture::create(ctx, {
    .type = r_texture_type::texture2d,
    .format = desc.format,
    .extent = desc.extent,
    .layers = 1,
    .levels = levels,
    .images = {desc},
    .gen_mipmaps = mips && levels > 1,
    .sampler = sampler,
    .addressing = addressing,
  });
}

inline r_expected<renderer_texture> r_make_texture2d(r_context_view ctx, const image_data& image,
                                                     r_texture_sampler sampler,
                                                     r_texture_address addressing,
                                                     uint32 levels = 7u, bool mips = true) {
  const auto desc = image.make_descriptor();
  return r_make_texture2d(ctx, desc, sampler, addressing, levels, mips);
}

template<tex_depth_type DepthT = uint8>
inline r_expected<renderer_texture> r_load_texture2d(r_context_view ctx, const std::string& path,
                                                     r_texture_sampler sampler,
                                                     r_texture_address addressing,
                                                     uint32 levels = 7u, bool mips = true) {
  return load_image<DepthT>(path)
  .and_then([&](auto&& image) -> r_expected<renderer_texture> {
    return r_make_texture2d(ctx, image, sampler, addressing, levels, mips);
  });
}

inline r_expected<renderer_shader> r_make_fragment_shader(r_context_view ctx,
                                                          std::string_view src) {
  return renderer_shader::create(ctx, {
    .type = r_shader_type::fragment,
    .source = {src}
  });
}

inline r_expected<renderer_shader> r_make_vertex_shader(r_context_view ctx,
                                                        std::string_view src) {
  return renderer_shader::create(ctx, {
    .type = r_shader_type::vertex,
    .source = {src}
  });
}

template<typename Vert>
requires(is_aos_vertex<Vert>)
r_expected<renderer_pipeline> r_make_pipeline(
  r_shader_view vert, r_shader_view frag,
  weak_cref<r_depth_test_opts> depth_test = r_def_depth_opts,
  weak_cref<r_blend_opts> blending = r_def_blending_opts
) {
  auto ctx = vert.context();
  const auto attribs = Vert::aos_binding();
  const r_shader stages[] = {vert.handle(), frag.handle()};
  return renderer_pipeline::create(ctx, {
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages,
    .primitive = r_primitive::triangles,
    .poly_mode = r_polygon_mode::fill,
    .poly_width = nullopt,
    .stencil_test = nullptr,
    .depth_test = depth_test,
    .scissor_test = nullptr,
    .face_culling = nullptr,
    .blending = blending,
  });
}

template<typename Vert>
requires(is_aos_vertex<Vert>)
r_expected<renderer_pipeline> r_make_pipeline(
  r_context_view ctx,
  std::string_view vert_src, std::string_view frag_src,
  weak_cref<r_depth_test_opts> depth_test = r_def_depth_opts,
  weak_cref<r_blend_opts> blending = r_def_blending_opts
) {
  auto vert = r_make_vertex_shader(ctx, vert_src);
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }
  auto frag = r_make_fragment_shader(ctx, frag_src);
  if (!frag) {
    return unexpected{std::move(frag.error())};
  }
  const auto attribs = Vert::aos_binding();
  const r_shader stages[] = {vert->handle(), frag->handle()};
  return renderer_pipeline::create(ctx, {
    .attributes = {attribs.data(), attribs.size()},
    .stages = stages,
    .primitive = r_primitive::triangles,
    .poly_mode = r_polygon_mode::fill,
    .poly_width = nullopt,
    .stencil_test = nullptr,
    .depth_test = depth_test,
    .scissor_test = nullptr,
    .face_culling = nullptr,
    .blending = blending,
  });
}

inline r_expected<std::pair<renderer_framebuffer, renderer_texture>> r_make_fbo(
  r_context_view ctx, const extent2d& extent,
  const color4& clear_color, r_clear_flag clear_flags,
  r_texture_sampler sampler = r_texture_sampler::nearest,
  r_texture_address addressing = r_texture_address::clamp_edge,
  r_texture_format format = r_texture_format::rgb8nu,
  r_test_buffer test_buffer = r_test_buffer::depth24u_stencil8u
) {
  return renderer_texture::create(ctx, {
    .type = r_texture_type::texture2d,
    .format = format,
    .extent = tex_extent_cast(extent),
    .layers = 1,
    .levels = 1,
    .images = {},
    .gen_mipmaps = false,
    .sampler = sampler,
    .addressing = addressing,
  })
  .and_then([&](auto&& tex) -> r_expected<std::pair<renderer_framebuffer, renderer_texture>> {
    const r_framebuffer_attachment fb_att {
      .texture = tex.handle(),
      .layer = 0u,
      .level = 0u,
    };
    auto fbo = renderer_framebuffer::create(ctx, {
      .extent = extent,
      .viewport = {0u, 0u, extent.x, extent.y},
      .clear_color = clear_color,
      .clear_flags = clear_flags,
      .test_buffer = test_buffer,
      .attachments = cspan<r_framebuffer_attachment>{fb_att},
    });
    if (!fbo) {
      return unexpected{std::move(fbo.error())};
    }
    return std::make_pair(std::move(*fbo), std::move(tex));
  });
}

inline r_expected<std::pair<renderer_window, renderer_context>> r_make_gl_ctx(
  uint32 win_width, uint32 win_height, const char* win_title,
  uint32 swap_interval = 0,
  r_clear_flag fb_clear = r_clear_flag::color_depth,
  const color4& fb_color = {.3f, .3f, .3f, 1.f}
) {
  const win_gl_params gl_params {
    .ver_major = 4,
    .ver_minor = 6,
  };
  return renderer_window::create({
    .width = win_width,
    .height = win_height,
    .title = win_title,
    .x11_class_name = win_title,
    .x11_instance_name = nullptr,
    .ctx_params = gl_params,
  })
  .and_then([&](auto&& win) -> r_expected<std::pair<renderer_window, renderer_context>> {
    const auto vp = uvec4{0, 0, win.fb_size()};
    auto ctx = renderer_context::create({
      .window = win.handle(),
      .renderer_api = win.renderer(),
      .swap_interval = swap_interval,
      .fb_viewport = vp,
      .fb_clear = fb_clear,
      .fb_color = fb_color,
      .alloc = nullptr,
    });
    if (!ctx) {
      return unexpected{std::move(ctx.error())};
    }
    return std::make_pair(std::move(win), std::move(*ctx));
  });
}

template<font_codepoint_type CodeT = char>
r_expected<font_renderer> r_load_font(
  r_context_view ctx,
  const std::string& path,
  font_charset_view<CodeT> charset = ascii_charset<CodeT>,
  uint32 glyph_size = 48u,
  uint32 padding = 0u,
  font_load_flags flags = font_load_flags::render_sdf,
  r_texture_sampler sampler = r_texture_sampler::linear,
  uint64 batch_size = 64u
) {
  return load_font_atlas(path, flags, glyph_size, padding, charset)
  .and_then([&](auto&& atlas) -> r_expected<font_renderer> {
    return font_renderer::create(ctx, std::move(atlas), sampler, batch_size);
  });
}

} // namespace ntf
