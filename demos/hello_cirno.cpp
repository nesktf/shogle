#include "shogle/render.hpp"
#include "shogle/assets.hpp"
#include "shogle/math.hpp"
#include "shogle/version.hpp"

static auto init_ctx() {
  ntf::r_win_gl_params gl_params {
    .ver_major = 4,
    .ver_minor = 6,
  };
  auto window = ntf::r_window::create({
    .width = 1280,
    .height = 720,
    .title = "test - hello_cirno - ShOGLE " SHOGLE_VERSION_STRING,
    .x11_class_name = "test",
    .x11_instance_name = nullptr,
    .ctx_params = &gl_params,
  });
  if (!window) {
    ntf::logger::error("Failed to create window: {}", window.error().what());
    std::exit(EXIT_FAILURE);
  }

  auto ctx = ntf::r_context::create({
    .window = *window,
    .use_api = ntf::r_api::opengl,
    .swap_interval = 0,
  });
  if (!ctx) {
    ntf::logger::error("Failed to initialize context: {}", ctx.error().what());
    std::exit(EXIT_FAILURE);
  }

  const ntf::color4 main_color{.3f, .3f, .3f, 1.f};
  const auto clear_flags = ntf::r_clear_flag::color_depth;
  const auto vp = ntf::uvec4{0, 0, window->fb_size()};

  ctx->framebuffer_color(ntf::r_context::DEFAULT_FRAMEBUFFER, main_color);
  ctx->framebuffer_clear(ntf::r_context::DEFAULT_FRAMEBUFFER, clear_flags);
  ctx->framebuffer_viewport(ntf::r_context::DEFAULT_FRAMEBUFFER, vp);;

  return std::make_pair(std::move(*window), std::move(*ctx));
}

int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  
  auto cousine = ntf::load_font_atlas<char>("./demos/res/CousineNerdFont-Regular.ttf");
  if (!cousine) {
    ntf::logger::error("[main] Failed to load font: {}", cousine.error().what());
    return EXIT_FAILURE;
  }

  const auto image_flag =
    ntf::image_load_flags::flip_y | ntf::image_load_flags::mark_normalized;
  auto cirno_img = ntf::load_image<ntf::uint8>("./demos/res/cirno_cpp.jpg", image_flag);
  if (!cirno_img) {
    ntf::logger::error("[main] Failed to load cirno image: {}", cirno_img.error().what());
    return EXIT_FAILURE;
  }

  auto atlas = ntf::load_atlas("./demos/res/2hus.json", ntf::atlas_load_flags::flip_y);
  if (!atlas) {
    ntf::logger::error("[main] Failed to load atlas: {}", atlas.error().what());
    return EXIT_FAILURE;
  }
  auto atlas_img = ntf::load_image<ntf::uint8>(atlas->image_path, image_flag);
  if (!atlas_img) {
    ntf::logger::error("[main] Failed to load atlas image: {}", atlas_img.error().what());
    return EXIT_FAILURE;
  }

  auto vert_src = *ntf::file_contents("./demos/res/shaders/vert_base.vs.glsl");
  auto frag_col_src = *ntf::file_contents("./demos/res/shaders/frag_color.fs.glsl");
  auto frag_tex_src = *ntf::file_contents("./demos/res/shaders/frag_tex.fs.glsl");
  auto vert_atl_src = *ntf::file_contents("./demos/res/shaders/vert_atlas.vs.glsl");
  auto frag_fnt_src = *ntf::file_contents("./demos/res/shaders/frag_font.fs.glsl");

  const auto fumo_flag = ntf::model_load_flags::triangulate;
  auto fumo = ntf::load_model<ntf::pnt_vertex>("./demos/res/cirno_fumo/cirno_fumo.obj", fumo_flag);
  if (!fumo) {
    ntf::logger::error("[main] Failed to load fumo model: {}", fumo.error().what());
    return EXIT_FAILURE;
  }

  auto fumo_diffuse = ntf::load_image<ntf::uint8>(fumo->materials.paths[0], image_flag);
  if (!fumo_diffuse) {
    ntf::logger::error("[main] Failed to load fumo material: {}", fumo_diffuse.error().what());
    return EXIT_FAILURE;
  }

  const auto& fumo_mesh = fumo->meshes.data[0];
  const auto& fumo_verts = fumo->meshes.vertices;
  const auto& fumo_inds = fumo->meshes.indices;

  auto [window, ctx] = init_ctx();
  
  auto font_img_data = cousine->make_bitmap_descriptor();
  auto font_tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = cousine->bitmap_format,
    .extent = ntf::tex_extent_cast(cousine->bitmap_extent),
    .layers = 1,
    .levels = 1,
    .images = {font_img_data},
    .gen_mipmaps = false,
    .sampler = ntf::r_texture_sampler::nearest,
    .addressing = ntf::r_texture_address::repeat,
  });

  auto cirno_img_data = cirno_img->make_descriptor();
  auto tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8nu,
    .extent = ntf::tex_extent_cast(cirno_img->dim),
    .layers = 1,
    .levels = 7,
    .images = {cirno_img_data},
    .gen_mipmaps = true,
    .sampler = ntf::r_texture_sampler::nearest,
    .addressing = ntf::r_texture_address::repeat,
  });

  auto atlas_img_data = atlas_img->make_descriptor();
  auto atlas_tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgba8nu,
    .extent = ntf::tex_extent_cast(atlas_img->dim),
    .layers = 1,
    .levels = 7,
    .images = {atlas_img_data},
    .gen_mipmaps = true,
    .sampler = ntf::r_texture_sampler::nearest,
    .addressing = ntf::r_texture_address::repeat,
  });

  ntf::r_buffer_data fumo_data[] {
    {
      .data = fumo_mesh.vertex_data(fumo_verts),
      .size = fumo_mesh.vertices_size(),
      .offset = 0,
    },
    {
      .data = fumo_mesh.index_data(fumo_inds),
      .size = fumo_mesh.indices_size(),
      .offset = 0,
    }
  };
  auto fumo_vbo = ntf::r_buffer::create(ntf::unchecked, ctx, {
    .type = ntf::r_buffer_type::vertex,
    .flags = ntf::r_buffer_flag::dynamic_storage,
    .size = fumo_mesh.vertices_size(),
    .data = &fumo_data[0],
  });

  auto fumo_ebo = ntf::r_buffer::create(ntf::unchecked, ctx, {
    .type = ntf::r_buffer_type::index,
    .flags = ntf::r_buffer_flag::dynamic_storage,
    .size = fumo_mesh.indices_size(),
    .data = &fumo_data[1],
  });

  auto fumo_img_data = fumo_diffuse->make_descriptor();
  auto fumo_tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8nu,
    .extent = ntf::tex_extent_cast(fumo_diffuse->dim),
    .layers = 1,
    .levels = 7,
    .images = {fumo_img_data},
    .gen_mipmaps = true,
    .sampler = ntf::r_texture_sampler::linear,
    .addressing = ntf::r_texture_address::repeat,
  });

  auto cube_vbo = ntf::r_buffer::create(ntf::unchecked, ctx, ntf::pnt_unindexed_cube_vert,
                                        ntf::r_buffer_type::vertex,
                                        ntf::r_buffer_flag::dynamic_storage);
  auto quad_vbo = ntf::r_buffer::create(ntf::unchecked, ctx, ntf::pnt_indexed_quad_vert,
                                        ntf::r_buffer_type::vertex,
                                        ntf::r_buffer_flag::dynamic_storage);
  auto quad_ebo = ntf::r_buffer::create(ntf::unchecked, ctx, ntf::pnt_indexed_quad_ind,
                                        ntf::r_buffer_type::index,
                                        ntf::r_buffer_flag::dynamic_storage);

  auto vertex = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::vertex,
    .source = {vert_src},
  });
  auto fragment_color = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::fragment,
    .source = {frag_col_src},
  });
  auto fragment_tex = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::fragment,
    .source = {frag_tex_src},
  });
  auto vertex_atlas = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::vertex,
    .source = {vert_atl_src},
  });
  auto fragment_fnt = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::fragment,
    .source = {frag_fnt_src},
  });

  auto load_pipeline = [&ctx](ntf::r_shader_handle vert, ntf::r_shader_handle frag) {
    auto attr_bind = ntf::pnt_vertex::attrib_binding();
    auto attr_desc = ntf::pnt_vertex::attrib_descriptor(); 

    ntf::r_shader_handle shads[] = {vert, frag};
    auto pipe = ntf::r_pipeline::create(ntf::unchecked, ctx, {
      .stages = {&shads[0], 2},
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
    return pipe;
  };
  auto pipe_col = load_pipeline(vertex.handle(), fragment_color.handle());
  auto pipe_tex = load_pipeline(vertex.handle(), fragment_tex.handle());
  auto pipe_atl = load_pipeline(vertex_atlas.handle(), fragment_tex.handle());
  auto pipe_fnt = load_pipeline(vertex.handle(), fragment_fnt.handle());

  auto fb_tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8nu,
    .extent = ntf::uvec3{1280, 720, 0},
    .layers = 1,
    .levels = 1,
    .images = {},
    .gen_mipmaps = false,
    .sampler = ntf::r_texture_sampler::nearest,
    .addressing = ntf::r_texture_address::repeat,
  });

  ntf::r_framebuffer_attachment fb_att {
    .handle = fb_tex.handle(),
    .layer = 0,
    .level = 0,
  };
  auto fbo = ntf::r_framebuffer::create(ntf::unchecked, ctx, {
    .extent = ntf::uvec2{1280, 720},
    .viewport = ntf::uvec4{0, 0, 1280, 720},
    .clear_color = ntf::color4{1.f, 0.f, 0.f, 1.f},
    .clear_flags = ntf::r_clear_flag::color_depth,
    .test_buffers = ntf::r_test_buffer_flag::both,
    .test_buffer_format = ntf::r_test_buffer_format::depth24u_stencil8u,
    .attachments = {fb_att},
    .color_buffer_format = {},
  });

  auto u_col_model = pipe_col.uniform(ntf::unchecked, "model");
  auto u_col_proj = pipe_col.uniform(ntf::unchecked, "proj");
  auto u_col_view = pipe_col.uniform(ntf::unchecked, "view");
  auto u_col_color = pipe_col.uniform(ntf::unchecked, "color");

  auto u_tex_model = pipe_tex.uniform(ntf::unchecked, "model");
  auto u_tex_proj = pipe_tex.uniform(ntf::unchecked, "proj");
  auto u_tex_view = pipe_tex.uniform(ntf::unchecked, "view");
  auto u_tex_color = pipe_tex.uniform(ntf::unchecked, "color");
  auto u_tex_sampler = pipe_tex.uniform(ntf::unchecked, "sampler0");

  auto u_atl_model = pipe_atl.uniform(ntf::unchecked, "model");
  auto u_atl_proj = pipe_atl.uniform(ntf::unchecked, "proj");
  auto u_atl_view = pipe_atl.uniform(ntf::unchecked, "view");
  auto u_atl_offset = pipe_atl.uniform(ntf::unchecked, "offset");
  auto u_atl_color = pipe_atl.uniform(ntf::unchecked, "color");
  auto u_atl_sampler = pipe_atl.uniform(ntf::unchecked, "sampler0");

  auto u_fnt_model = pipe_fnt.uniform(ntf::unchecked, "model");
  auto u_fnt_proj = pipe_fnt.uniform(ntf::unchecked, "proj");
  auto u_fnt_view = pipe_fnt.uniform(ntf::unchecked, "view");
  auto u_fnt_color = pipe_fnt.uniform(ntf::unchecked, "text_color");
  auto u_fnt_sampler = pipe_fnt.uniform(ntf::unchecked, "sampler0");
  auto u_fnt_time = pipe_fnt.uniform(ntf::unchecked, "time");

  ntf::float32 fb_ratio = 1280.f/720.f;
  auto transf_cube = ntf::transform3d<ntf::float32>{}
    .pos(0.f, 0.f, 0.f).scale(1.f);
  ntf::mat4 cam_view_cube = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -3.f});
  ntf::mat4 cam_proj_fumo = glm::perspective(glm::radians(45.f), fb_ratio, .1f, 100.f);
  ntf::vec4 color_cube {0.f, 1.f, 0.f, 1.f};

  auto transf_quad0 = ntf::transform2d<ntf::float32>{}
    .pos(-350.f, 0.f).scale(ntf::vec2{300.f, 300.f});
  auto transf_quad1 = ntf::transform2d<ntf::float32>{}
    .pos(350.f, 0.f).scale(ntf::vec2{fb_ratio*300.f, 300.f});
  ntf::mat4 cam_view_quad = glm::translate(glm::mat4{1.f}, glm::vec3{640.f, 360.f, -3.f});
  ntf::mat4 cam_proj_quad = glm::ortho(0.f, 1280.f, 0.f, 720.f, .1f, 100.f);
  auto cam_proj_cube = cam_proj_fumo;
  ntf::vec4 color_quad {1.f, 1.f, 1.f, 1.f};

  auto ext = cousine->bitmap_extent;
  float fnt_scale = 500.f;
  float fnt_aspect = (float)ext.x/(float)ext.y;
  auto transf_font = ntf::transform2d<ntf::float32>{}
    .pos(0.f, 0.f).scale(fnt_aspect*fnt_scale, -fnt_scale);
  ntf::vec4 color_fnt{1.f, 0.f, 0.f, 1.f};

  const ntf::float32 fumo_scale = 0.04f;
  auto transf_fumo = ntf::transform3d<ntf::float32>{}
    .pos(0.f, -.3f, 0.f).scale(fumo_scale);

  const ntf::float32 rin_scale = 300.f;
  const auto& rin_seq = atlas->sequence_at(*atlas->find_sequence("rin.dance"));
  auto rin_base_index = rin_seq.entries.index;
  auto rin_count = rin_seq.entries.count;

  const ntf::vec4* rin_uvs = &atlas->sprites[atlas->indices[rin_base_index]].offset;
  const auto rin_aspect = atlas->sprites[atlas->indices[rin_base_index]].aspect();
  // const ntf::uvec2* rin_dim = &atlas.sprites[rin_seq.entries.index].dim;

  auto transf_rin = ntf::transform2d<ntf::float32>{}
    .pos(0.f, -200.f).scale(rin_scale*rin_aspect, -rin_scale);

  ntf::float32 t = 0;
  ntf::float64 avg_fps{0};
  ntf::float64 fps[120] = {0};
  ntf::uint8 fps_counter{0};
  ntf::float32 t2 = 0;
  ntf::uint32 ups = 60;
  ntf::uint32 rin_counter = 0;
  ntf::uint32 rin_curr_index = rin_base_index;

  bool do_things = true;

  window.key_event([&](ntf::r_window& win, ntf::keycode code, auto, ntf::keystate state, auto) {
    const float ts = 4.f;
    if (state == ntf::keystate::press) {
      if (code == ntf::keycode::key_escape) {
        win.close();
      }
      if (code == ntf::keycode::key_space) {
        do_things = !do_things;
      }

      if (code == ntf::keycode::key_left) {
        t -= ts/static_cast<float>(ups);
      } else if (code == ntf::keycode::key_right) {
        t += ts/static_cast<float>(ups);
      }
    }
  });

  window.viewport_event([&](ntf::r_window&, ntf::uint32 w, ntf::uint32 h) {
    ctx.framebuffer_viewport(ntf::r_context::DEFAULT_FRAMEBUFFER, ntf::uvec4{0, 0, w, h});
    fb_ratio = static_cast<ntf::float32>(w)/static_cast<ntf::float32>(h);
    cam_proj_fumo = glm::perspective(glm::radians(45.f), fb_ratio, .1f, 100.f);
  });

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  ntf::shogle_render_loop(window, ctx, ups, ntf::overload{
    [&](ntf::uint32 ups) {
      if (do_things) {
        ntf::float32 dt = 1/static_cast<ntf::float32>(ups);
        t += glm::pi<float>()*dt;
      }

      if (++rin_counter == 3) {
        rin_curr_index = ((rin_curr_index-rin_base_index+1) % rin_count)+rin_base_index;
        rin_uvs = &atlas->sprites[atlas->indices[rin_curr_index]].offset;
        rin_counter = 0;
      }

      transf_cube
        .rot(t, ntf::vec3{0.f, 1.f, 0.f})
        .scale_y(.5f + std::abs(std::sin(t)))
        .offset_y(-0.5f);
      transf_quad0.roll(-t);
      transf_quad1.roll(t);
      transf_fumo
        .rot(glm::pi<ntf::float32>()*.5f*t, ntf::vec3{0.f, 1.f, 0.f})
        .scale_y(fumo_scale*.5f+fumo_scale*.5f*std::abs(std::sin(glm::pi<ntf::float32>()*t)));
    },
    [&](ntf::float64 dt, ntf::float64) {
      // Using an ode solver instead of t+=dt just because
      t2 = ntf::ode_euler<ntf::float32>{}(0.f, t2, dt, [](...) { return 1.f; });

      if (t2 > 0.016*.5) {
        fps[fps_counter] = 1/dt;
        fps_counter++;
        t2 = 0.f;
      }
      if (fps_counter > 120) {
        fps_counter = 0;
        avg_fps = 0;
        for (ntf::uint8 i = 0; i < 120; ++i) {
          avg_fps += fps[i];
        }
        avg_fps /= 120.f;
      }

      const ntf::uint32 ups_min = 1;
      const ntf::uint32 ups_max = 85;
      const auto gui_flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
      ImGui::Begin("test", nullptr, gui_flags);
        ImGui::SetWindowPos(ImVec2{0, 0});
        ImGui::SetWindowSize(ImVec2{250, 100});
        ImGui::Text("Delta time: %f", dt);
        ImGui::Text("Avg fps: %f", avg_fps);
        ImGui::Text("FDelta time: %f", 1/static_cast<float>(ups));
        ImGui::SliderScalar("ups", ImGuiDataType_U32, &ups, &ups_min, &ups_max);
      ImGui::End();
      ImGui::ShowDemoWindow();


      // Buffer bindings
      const ntf::r_buffer_binding fumo_bbind[] = {
        {fumo_vbo.handle(), ntf::r_buffer_type::vertex, ntf::nullopt},
        {fumo_ebo.handle(), ntf::r_buffer_type::index, ntf::nullopt},
      };
      const ntf::r_buffer_binding quad_bbind[] = {
        {quad_vbo.handle(), ntf::r_buffer_type::vertex, ntf::nullopt},
        {quad_ebo.handle(), ntf::r_buffer_type::index, ntf::nullopt},
      };
      const ntf::r_buffer_binding cube_bbind[] = {
        {cube_vbo.handle(), ntf::r_buffer_type::vertex, ntf::nullopt},
      };

      // Texture bindings
      const ntf::r_texture_binding fumo_tbind[] = {
        {.texture = fumo_tex.handle(), .location = 0},
      };
      const ntf::r_texture_binding cino_tbind[] = {
        {.texture = tex.handle(), .location = 0},
      };
      const ntf::r_texture_binding rin_tbind[] = {
        {.texture = atlas_tex.handle(), .location = 0},
      };
      const ntf::r_texture_binding fb_tbind[] = {
        {.texture = fb_tex.handle(), .location = 0},
      };
      const ntf::r_texture_binding fnt_tbind[] = {
        {.texture = font_tex.handle(), .location = 0},
      };

      // Uniforms
      const ntf::r_push_constant fumo_unifs[] = {
        ntf::r_format_pushconst(u_tex_model, transf_fumo.world()),
        ntf::r_format_pushconst(u_tex_proj, cam_proj_fumo),
        ntf::r_format_pushconst(u_tex_view, cam_view_cube),
        ntf::r_format_pushconst(u_tex_color, color_quad),
        ntf::r_format_pushconst(u_tex_sampler, 0),
      };
      const ntf::r_push_constant cino_unifs[] = {
        ntf::r_format_pushconst(u_tex_model, transf_quad0.world()),
        ntf::r_format_pushconst(u_tex_proj, cam_proj_quad),
        ntf::r_format_pushconst(u_tex_view, cam_view_quad),
        ntf::r_format_pushconst(u_tex_color, color_quad),
        ntf::r_format_pushconst(u_tex_sampler, 0),
      };
      const ntf::r_push_constant fb_unifs[] = {
        ntf::r_format_pushconst(u_tex_model, transf_quad1.world()),
        ntf::r_format_pushconst(u_tex_proj, cam_proj_quad),
        ntf::r_format_pushconst(u_tex_view, cam_view_quad),
        ntf::r_format_pushconst(u_tex_color, color_quad),
        ntf::r_format_pushconst(u_tex_sampler, 0),
      };
      const ntf::r_push_constant rin_unifs[] = {
        ntf::r_format_pushconst(u_atl_model, transf_rin.world()),
        ntf::r_format_pushconst(u_atl_proj, cam_proj_quad),
        ntf::r_format_pushconst(u_atl_view, cam_view_quad),
        ntf::r_format_pushconst(u_atl_color, color_quad),
        ntf::r_format_pushconst(u_atl_offset, *rin_uvs),
        ntf::r_format_pushconst(u_atl_sampler, 0),
      };
      const ntf::r_push_constant cube_unifs[] = {
        ntf::r_format_pushconst(u_col_model, transf_cube.world()),
        ntf::r_format_pushconst(u_col_proj, cam_proj_cube),
        ntf::r_format_pushconst(u_col_view, cam_view_cube),
        ntf::r_format_pushconst(u_col_color, color_cube),
      };
      const ntf::r_push_constant fnt_unifs[] = {
        ntf::r_format_pushconst(u_fnt_model, transf_font.world()),
        ntf::r_format_pushconst(u_fnt_proj, cam_proj_quad),
        ntf::r_format_pushconst(u_fnt_view, cam_view_quad),
        ntf::r_format_pushconst(u_fnt_color, color_fnt),
        ntf::r_format_pushconst(u_fnt_sampler, 0),
        ntf::r_format_pushconst(u_fnt_time, .25f*t),
      };

      ntf::r_draw_opts fumo_opts {
        .count = static_cast<ntf::uint32>(fumo_mesh.indices.count),
        .offset = 0,
        .instances = 0,
        .sort_group = 0,
      };
      ntf::r_draw_opts quad_opts {
        .count = 6,
        .offset = 0,
        .instances = 0,
        .sort_group = 0,
      };
      ntf::r_draw_opts cube_opts {
        .count = 36,
        .offset = 0,
        .instances = 0,
        .sort_group = 0,
      };

      // Fumo
      ctx.submit({
        .target = ntf::r_context::DEFAULT_FRAMEBUFFER,
        .pipeline = pipe_tex.handle(),
        .buffers = {&fumo_bbind[0], std::size(fumo_bbind)},
        .textures = {&fumo_tbind[0], std::size(fumo_tbind)},
        .uniforms = {&fumo_unifs[0], std::size(fumo_unifs)},
        .draw_opts = fumo_opts,
        .on_render = {},
      });

      // // Cirno quad
      // ctx.submit({
      //   .target = ntf::r_context::DEFAULT_FRAMEBUFFER,
      //   .pipeline = pipe_tex.handle(),
      //   .buffers = {&quad_bbind[0], std::size(quad_bbind)},
      //   .textures = {&cino_tbind[0], std::size(cino_tbind)},
      //   .uniforms = {&cino_unifs[0], std::size(cino_unifs)},
      //   .draw_opts = quad_opts,
      //   .on_render = {},
      // });

      // Rin sprite
      ctx.submit({
        .target = ntf::r_context::DEFAULT_FRAMEBUFFER,
        .pipeline = pipe_atl.handle(),
        .buffers = {&quad_bbind[0], std::size(quad_bbind)},
        .textures = {&rin_tbind[0], std::size(rin_tbind)},
        .uniforms = {&rin_unifs[0], std::size(rin_unifs)},
        .draw_opts = quad_opts,
        .on_render = {},
      });

      // // Framebuffer viewport
      // ctx.submit({
      //   .target = ntf::r_context::DEFAULT_FRAMEBUFFER,
      //   .pipeline = pipe_tex.handle(),
      //   .buffers = {&quad_bbind[0], std::size(quad_bbind)},
      //   .textures = {&fb_tbind[0], std::size(fb_tbind)},
      //   .uniforms = {&fb_unifs[0], std::size(fb_unifs)},
      //   .draw_opts = quad_opts,
      //   .on_render = {},
      // });;

      // Font thing
      ctx.submit({
        .target = ntf::r_context::DEFAULT_FRAMEBUFFER,
        .pipeline = pipe_fnt.handle(),
        .buffers = {&quad_bbind[0], std::size(quad_bbind)},
        .textures = {&fnt_tbind[0], std::size(fnt_tbind)},
        .uniforms = {&fnt_unifs[0], std::size(fnt_unifs)},
        .draw_opts = quad_opts,
        .on_render = {},
      });

      // Cube
      ctx.submit({
        .target = fbo.handle(),
        .pipeline = pipe_col.handle(),
        .buffers = {&cube_bbind[0], std::size(cube_bbind)},
        .textures = {},
        .uniforms = {&cube_unifs[0], std::size(cube_unifs)},
        .draw_opts = cube_opts,
        .on_render = {},
      });

      // Other Cirno quad
      ctx.submit({
        .target = fbo.handle(),
        .pipeline = pipe_tex.handle(),
        .buffers = {&quad_bbind[0], std::size(quad_bbind)},
        .textures = {&cino_tbind[0], std::size(cino_tbind)},
        .uniforms = {&cino_unifs[0], std::size(cino_unifs)},
        .draw_opts = quad_opts,
        .on_render = {},
      });
    }
  });

  return EXIT_SUCCESS;
}
