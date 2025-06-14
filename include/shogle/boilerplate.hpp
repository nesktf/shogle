#pragma once

#include "./render.hpp"
#include "./assets.hpp"
#include "./scene.hpp"

#include <ntfstl/utility.hpp>

namespace ntf::render {

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
void render_loop(window& win, context_view ctx, LoopObj&& obj) {
  using namespace std::literals;

  using clock = std::chrono::steady_clock;
  using duration = clock::duration;
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntfr::render_loop] Main loop started");

  time_point last_time = clock::now();
  while (!win.should_close()) {
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};

    win.poll_events();

    ctx.start_frame();
    if constexpr (has_render_member<LoopObj>) {
      obj.on_render(dt);
    } else {
      obj(dt);
    }
    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntfr::render_loop] Main loop exit");
}

template<fixed_loop_object LoopObj>
void render_loop(window& win, context_view ctx, const uint32& ups, LoopObj&& obj) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntfr::render_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!win.should_close()) {
    fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    win.poll_events();

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

  SHOGLE_LOG(debug, "[ntfr::render_loop] Main loop exit");
}

template<fixed_render_func RFunc, fixed_update_func UFunc>
void render_loop(window& win, context_view ctx, const uint32& ups,
                 RFunc&& render, UFunc&& fixed_update) {
  using namespace std::literals;

  auto fixed_elapsed_time =
    std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};

  using clock = std::chrono::steady_clock;
  using duration = decltype(clock::duration{} + fixed_elapsed_time);
  using time_point = std::chrono::time_point<clock, duration>;

  SHOGLE_LOG(debug, "[ntfr::render_loop] Fixed main loop started at {} ups", ups);

  time_point last_time = clock::now();
  duration lag = 0s;
  while (!win.should_close()) {
    fixed_elapsed_time = std::chrono::duration<double>{std::chrono::microseconds{1000000/ups}};
    time_point start_time = clock::now();
    auto elapsed_time = start_time - last_time;
    last_time = start_time;
    lag += elapsed_time;

    double dt {std::chrono::duration<double>{elapsed_time}/1s};
    double alpha {std::chrono::duration<double>{lag}/fixed_elapsed_time};

    win.poll_events();

    ctx.start_frame();

    while (lag >= fixed_elapsed_time) {
      fixed_update(ups);
      lag -= fixed_elapsed_time;
    }

    render(dt, alpha);

    ctx.end_frame();
  }
  ctx.device_wait();

  SHOGLE_LOG(debug, "[ntfr::render_loop] Main loop exit");
}

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

inline expect<texture2d> make_texture2d(context_view ctx, const ntf::bitmap_data& image,
                                        texture_sampler sampler,
                                        texture_addressing addressing,
                                        uint32 levels = 7u, bool mips = true) {
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

inline expect<texture2d> make_texture2d(context_view ctx, image_format format, extent2d extent,
                                        texture_sampler sampler,
                                        texture_addressing addressing,
                                        uint32 levels = 7u) {
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
inline expect<texture2d> load_texture2d(context_view ctx, const std::string& path,
                                        texture_sampler sampler,
                                        texture_addressing addressing,
                                        uint32 levels = 7u, bool mips = true) {
  return load_image<DepthT>(path)
  .and_then([&](auto&& image) -> expect<texture2d> {
    return make_texture2d(ctx, image, sampler, addressing, levels, mips);
  });
}

template<typename Vert>
requires(meta::is_aos_vertex<Vert>)
expect<pipeline> make_pipeline(
  vertex_shader_view vert, fragment_shader_view frag,
  weak_cptr<depth_test_opts> depth_test = def_depth_opts,
  weak_cptr<blend_opts> blending = def_blending_opts
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
expect<pipeline> make_pipeline(
  context_view ctx,
  std::string_view vert_src, std::string_view frag_src,
  weak_cptr<depth_test_opts> depth_test = def_depth_opts,
  weak_cptr<blend_opts> blending = def_blending_opts
) {
  auto vert = vertex_shader::create(ctx, {vert_src});
  if (!vert) {
    return unexpected{std::move(vert.error())};
  }
  auto frag = fragment_shader::create(ctx, {frag_src});
  if (!frag) {
    return unexpected{std::move(frag.error())};
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

inline expect<std::pair<framebuffer, texture2d>> make_fbo(
  context_view ctx, const extent2d& extent,
  const color4& clear_color, clear_flag clear_flags,
  texture_sampler sampler = texture_sampler::nearest,
  texture_addressing addressing = texture_addressing::clamp_edge,
  image_format format = image_format::rgb8nu,
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
  .and_then([&](auto&& tex) -> expect<std::pair<framebuffer, texture2d>> {
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
      return unexpected{std::move(fbo.error())};
    }
    return std::make_pair(std::move(*fbo), std::move(tex));
  });
}

inline expect<std::pair<window, context>> make_gl_ctx(
  uint32 win_width, uint32 win_height, const char* win_title,
  uint32 swap_interval = 0,
  clear_flag fb_clear = clear_flag::color_depth,
  const color4& fb_color = {.3f, .3f, .3f, 1.f}
) {
  return window::create({
    .width = win_width,
    .height = win_height,
    .title = win_title,
    .x11 = nullptr,
    .ver_major = 4,
    .ver_minor = 6,
    .fb_msaa_level = 8,
    .fb_buffer = ntfr::fbo_buffer::depth24u_stencil8u,
  })
  .and_then([&](auto&& win) -> expect<std::pair<window, context>> {
    const auto vp = uvec4{0, 0, win.fb_size()};
    auto ctx = context::create({
      .window = win.get(),
      .ctx_api = win.renderer(),
      .swap_interval = swap_interval,
      .fb_viewport = vp,
      .fb_clear_flags = fb_clear,
      .fb_clear_color = fb_color,
      .alloc = nullptr,
    });
    if (!ctx) {
      return unexpected{std::move(ctx.error())};
    }
    return std::make_pair(std::move(win), std::move(*ctx));
  });
}

template<font_codepoint_type CodeT = char>
expect<ntfr::font_renderer> r_load_font(
  context_view ctx,
  const std::string& path,
  font_charset_view<CodeT> charset = ascii_charset<CodeT>,
  uint32 glyph_size = 48u,
  uint32 padding = 0u,
  font_load_flags flags = font_load_flags::render_sdf,
  texture_sampler sampler = texture_sampler::linear,
  uint64 batch_size = 64u
) {
  return load_font_atlas(path, flags, glyph_size, padding, charset)
  .and_then([&](auto&& atlas) -> expect<ntfr::font_renderer> {
    return font_renderer::create(ctx, std::move(atlas), sampler, batch_size);
  });
}

} // namespace ntf::render
