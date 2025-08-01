#include <shogle/math.hpp>
#include <shogle/version.hpp>
#include <shogle/boilerplate.hpp>

int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  
  auto cousine = ntf::load_font_atlas<char>("./demos/res/CousineNerdFont-Regular.ttf");
  if (!cousine) {
    ntf::logger::error("[main] Failed to load font: {}", cousine.error().what());
    return EXIT_FAILURE;
  }

  auto cirno_img = ntf::load_image<ntf::uint8>("./demos/res/cirno_cpp.jpg");
  if (!cirno_img) {
    ntf::logger::error("[main] Failed to load cirno image: {}", cirno_img.error().what());
    return EXIT_FAILURE;
  }

  auto atlas = ntf::load_atlas("./demos/res/2hus.json", ntf::atlas_load_flags::flip_y);
  if (!atlas) {
    ntf::logger::error("[main] Failed to load atlas: {}", atlas.error().what());
    return EXIT_FAILURE;
  }
  auto atlas_img = ntf::load_image<ntf::uint8>(atlas->image_path);
  if (!atlas_img) {
    ntf::logger::error("[main] Failed to load atlas image: {}", atlas_img.error().what());
    return EXIT_FAILURE;
  }

  auto vert_src = ntf::file_contents("./demos/res/shaders/vert_base.vs.glsl").value();
  auto frag_col_src = ntf::file_contents("./demos/res/shaders/frag_color.fs.glsl").value();
  auto frag_tex_src = ntf::file_contents("./demos/res/shaders/frag_tex.fs.glsl").value();
  auto vert_atl_src = ntf::file_contents("./demos/res/shaders/vert_atlas.vs.glsl").value();

  const auto fumo_flag = ntf::model_load_flags::triangulate;
  auto fumo = ntf::load_model<ntfr::pnt_vertex>("./demos/res/cirno_fumo/cirno_fumo.obj", fumo_flag);
  if (!fumo) {
    ntf::logger::error("[main] Failed to load fumo model: {}", fumo.error().what());
    return EXIT_FAILURE;
  }

  auto fumo_diffuse = ntf::load_image<ntf::uint8>(fumo->materials.paths[0]);
  if (!fumo_diffuse) {
    ntf::logger::error("[main] Failed to load fumo material: {}", fumo_diffuse.error().what());
    return EXIT_FAILURE;
  }

  const auto& fumo_mesh = fumo->meshes.data[0];
  const auto& fumo_verts = fumo->meshes.vertices;
  const auto& fumo_inds = fumo->meshes.indices;

  const char win_title[] = "test - hello_cirno - " SHOGLE_VERSION;
  auto [window, ctx] = ntfr::make_gl_ctx(1280, 720, win_title).value();
  auto imgui = ntfr::imgui_ctx::create(ctx.get(), window.get());

  auto quad = ntfr::quad_mesh::create(ctx).value();
  auto cube = ntfr::cube_mesh::create(ctx).value();

  ntfr::text_buffer text_buffer;
  ntf::mat4 cam_proj_fnt = glm::ortho(0.f, 1280.f, 0.f, 720.f);
  auto sdf_rule = ntfr::sdf_text_rule::create(ctx,
                                             ntfr::color3{1.f, 0.f, 0.f}, 0.5f, 0.05f,
                                             ntfr::color3{0.f, 0.f, 0.f},
                                             ntf::vec2{-0.005f, -0.005f},
                                             0.62f, 0.05f);
  if (!sdf_rule) {
    ntf::logger::error("[main] Failed to create text render rule: {}", sdf_rule.error().what());
    return EXIT_FAILURE;
  }

  auto frenderer = ntfr::font_renderer::create(ctx, cam_proj_fnt, std::move(*cousine));
  if (!frenderer) {
    ntf::logger::error("[main] Failed to create font renderer: {}", frenderer.error().what());
    return EXIT_FAILURE;
  }

  auto tex = ntfr::make_texture2d(ctx, *cirno_img,
                                  ntfr::texture_sampler::nearest,
                                  ntfr::texture_addressing::repeat).value();
  auto atlas_tex = ntfr::make_texture2d(ctx, *atlas_img,
                                        ntfr::texture_sampler::nearest,
                                        ntfr::texture_addressing::repeat).value();
  auto fumo_tex = ntfr::make_texture2d(ctx, *fumo_diffuse,
                                       ntfr::texture_sampler::linear,
                                       ntfr::texture_addressing::repeat).value();

  const ntfr::buffer_data fumo_vbo_data {
    .data = fumo_mesh.vertex_data(fumo_verts),
    .size = fumo_mesh.vertices_size(),
    .offset = 0u,
  };
  auto fumo_vbo = ntfr::vertex_buffer::create(ctx, {
    .flags = ntfr::buffer_flag::dynamic_storage,
    .size = fumo_mesh.vertices_size(),
    .data = fumo_vbo_data,
  }).value();

  const ntfr::buffer_data fumo_ebo_data {
    .data = fumo_mesh.index_data(fumo_inds),
    .size = fumo_mesh.indices_size(),
    .offset = 0u,
  };
  auto fumo_ebo = ntfr::index_buffer::create(ctx, {
    .flags = ntfr::buffer_flag::dynamic_storage,
    .size = fumo_mesh.indices_size(),
    .data = fumo_ebo_data,
  }).value();


  auto vertex = ntfr::vertex_shader::create(ctx, {vert_src}).value();
  auto vertex_atlas = ntfr::vertex_shader::create(ctx, {vert_atl_src}).value();
  auto fragment_color = ntfr::fragment_shader::create(ctx, {frag_col_src}).value();
  auto fragment_tex = ntfr::fragment_shader::create(ctx, {frag_tex_src}).value();

  auto pipe_col = ntfr::make_pipeline<ntfr::pnt_vertex>(vertex, fragment_color).value();
  auto pipe_tex = ntfr::make_pipeline<ntfr::pnt_vertex>(vertex, fragment_tex).value();
  auto pipe_atl = ntfr::make_pipeline<ntfr::pnt_vertex>(vertex_atlas, fragment_tex).value();

  auto [fbo, fbo_tex] = ntfr::make_fbo(ctx, {1280, 720}, {1.f, 0.f, 0.f, 1.f},
                                        ntfr::clear_flag::color_depth).value();

  auto u_col_model = pipe_col.uniform("model");
  auto u_col_proj = pipe_col.uniform("proj");
  auto u_col_view = pipe_col.uniform("view");
  auto u_col_color = pipe_col.uniform("color");

  auto u_tex_model = pipe_tex.uniform("model");
  auto u_tex_proj = pipe_tex.uniform("proj");
  auto u_tex_view = pipe_tex.uniform("view");
  auto u_tex_color = pipe_tex.uniform("color");
  auto u_tex_sampler = pipe_tex.uniform("sampler0");

  auto u_atl_model = pipe_atl.uniform("model");
  auto u_atl_proj = pipe_atl.uniform("proj");
  auto u_atl_view = pipe_atl.uniform("view");
  auto u_atl_offset = pipe_atl.uniform("offset");
  auto u_atl_color = pipe_atl.uniform("color");
  auto u_atl_sampler = pipe_atl.uniform("sampler0");

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
  float font_scale = 1.f;

  bool do_things = true;

  window.set_key_press_callback([&](ntfr::window& win, const ntfr::win_key_data& key_data) {
    const float ts = 4.f;
    if (key_data.action == ntfr::win_action::press) {
      if (key_data.key == ntfr::win_key::escape) {
        win.close();
      }
      if (key_data.key == ntfr::win_key::space) {
        do_things = !do_things;
      }

      if (key_data.key == ntfr::win_key::left) {
        t -= ts/static_cast<float>(ups);
      } else if (key_data.key == ntfr::win_key::right) {
        t += ts/static_cast<float>(ups);
      }
    }
  });

  const auto default_fbo = ntfr::framebuffer::get_default(ctx);
  window.set_viewport_callback([&](auto&, const ntfr::extent2d& extent) {
    ntf::uint32 w = extent.x;
    ntf::uint32 h = extent.y;
    default_fbo.viewport(ntf::uvec4{0, 0, w, h});
    fb_ratio = static_cast<ntf::float32>(w)/static_cast<ntf::float32>(h);
    cam_proj_fumo = glm::perspective(glm::radians(45.f), fb_ratio, .1f, 100.f);
    // cam_proj_quad = glm::ortho(0.f, (float)w, 0.f, (float)h, .1f, 100.f);
    // cam_proj_fnt = glm::ortho(0.f, (float)w, 0.f, (float)h);
  });

  ntfr::render_loop(window, ctx, ups, ntf::overload{
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

      imgui.start_frame();
      const ntf::uint32 ups_min = 1;
      const ntf::uint32 ups_max = 85;
      const auto gui_flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
      ImGui::Begin("test", nullptr, gui_flags);
        ImGui::SetWindowPos(ImVec2{0, 0});
        ImGui::SetWindowSize(ImVec2{250, 110});
        ImGui::Text("Delta time: %f", dt);
        ImGui::Text("Avg fps: %f", avg_fps);
        ImGui::Text("FDelta time: %f", 1/static_cast<float>(ups));
        ImGui::SliderScalar("ups", ImGuiDataType_U32, &ups, &ups_min, &ups_max);
        ImGui::SliderFloat("font scale: ", &font_scale, 1.f, 8.f);
      ImGui::End();
      ImGui::ShowDemoWindow();
      imgui.end_frame(default_fbo.get(), 1u);


      // Buffer bindings
      const auto quad_bbind = quad.bindings();
      const auto cube_bbind = cube.bindings();

      // Uniforms
      const ntfr::uniform_const fumo_unifs[] = {
        ntfr::format_uniform_const(u_tex_model, transf_fumo.world()),
        ntfr::format_uniform_const(u_tex_proj, cam_proj_fumo),
        ntfr::format_uniform_const(u_tex_view, cam_view_cube),
        ntfr::format_uniform_const(u_tex_color, color_quad),
        ntfr::format_uniform_const(u_tex_sampler, 0),
      };
      const ntfr::uniform_const cino_unifs[] = {
        ntfr::format_uniform_const(u_tex_model, transf_quad0.world()),
        ntfr::format_uniform_const(u_tex_proj, cam_proj_quad),
        ntfr::format_uniform_const(u_tex_view, cam_view_quad),
        ntfr::format_uniform_const(u_tex_color, color_quad),
        ntfr::format_uniform_const(u_tex_sampler, 0),
      };
      const ntfr::uniform_const fb_unifs[] = {
        ntfr::format_uniform_const(u_tex_model, transf_quad1.world()),
        ntfr::format_uniform_const(u_tex_proj, cam_proj_quad),
        ntfr::format_uniform_const(u_tex_view, cam_view_quad),
        ntfr::format_uniform_const(u_tex_color, color_quad),
        ntfr::format_uniform_const(u_tex_sampler, 0),
      };
      const ntfr::uniform_const rin_unifs[] = {
        ntfr::format_uniform_const(u_atl_model, transf_rin.world()),
        ntfr::format_uniform_const(u_atl_proj, cam_proj_quad),
        ntfr::format_uniform_const(u_atl_view, cam_view_quad),
        ntfr::format_uniform_const(u_atl_color, color_quad),
        ntfr::format_uniform_const(u_atl_offset, *rin_uvs),
        ntfr::format_uniform_const(u_atl_sampler, 0),
      };
      const ntfr::uniform_const cube_unifs[] = {
        ntfr::format_uniform_const(u_col_model, transf_cube.world()),
        ntfr::format_uniform_const(u_col_proj, cam_proj_cube),
        ntfr::format_uniform_const(u_col_view, cam_view_cube),
        ntfr::format_uniform_const(u_col_color, color_cube),
      };

      const ntfr::render_opts fumo_opts {
        .vertex_count = static_cast<ntf::uint32>(fumo_mesh.indices.count),
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      };
      const ntfr::render_opts quad_opts {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      };
      const ntfr::render_opts cube_opts {
        .vertex_count = 36,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      };

      const ntfr::vertex_binding fumo_vertex_bind[] {
        {.buffer = fumo_vbo, .layout = 0u},
        {.buffer = fumo_vbo, .layout = 1u},
        {.buffer = fumo_vbo, .layout = 2u},
      };

      // Fumo
      const ntfr::texture_binding fumo_tex_bind{.texture = fumo_tex, .sampler = 0};
      ctx.submit_render_command({
        .target = default_fbo,
        .pipeline = pipe_tex,
        .buffers = {
          .vertex = fumo_vertex_bind,
          .index = fumo_ebo,
          .shader = {},
        },
        .textures = {fumo_tex_bind},
        .consts = fumo_unifs,
        .opts = fumo_opts,
        .sort_group = 0u,
        .render_callback = {},
      });

      // Rin sprite
      const ntfr::texture_binding rin_tex_bind{.texture = atlas_tex, .sampler = 0};
      ctx.submit_render_command({
        .target = default_fbo,
        .pipeline = pipe_atl,
        .buffers = quad_bbind,
        .textures = {rin_tex_bind},
        .consts = rin_unifs,
        .opts = quad_opts,
        .sort_group = 0u,
        .render_callback = {},
      });

      // Framebuffer viewport
      const ntfr::texture_binding fbo_tex_bind{.texture = fbo_tex, .sampler = 0};
      ctx.submit_render_command({
        .target = default_fbo,
        .pipeline = pipe_tex,
        .buffers = quad_bbind,
        .textures = {fbo_tex_bind},
        .consts = fb_unifs,
        .opts = quad_opts,
        .sort_group = 0u,
        .render_callback = {},
      });

      // Cube
      ctx.submit_render_command({
        .target = fbo,
        .pipeline = pipe_col,
        .buffers = cube_bbind,
        .textures = {},
        .consts = cube_unifs,
        .opts = cube_opts,
        .sort_group = 0u,
        .render_callback = {},
      });

      // Cirno quad
      const ntfr::texture_binding cino_tex_bind{.texture = tex, .sampler = 0};
      ctx.submit_render_command({
        .target = fbo,
        .pipeline = pipe_tex,
        .buffers = quad_bbind,
        .textures = {cino_tex_bind},
        .consts = cino_unifs,
        .opts = quad_opts,
        .sort_group = 0u,
        .render_callback = {},
      });

      text_buffer.clear();
      text_buffer.append_fmt(frenderer->glyphs(), 40.f, 400.f, font_scale,
                             "Hello World! ~ze\n{:.2f}fps - {:.2f}ms", avg_fps, 1000/avg_fps);
      frenderer->render(quad, default_fbo, *sdf_rule, text_buffer, 1u);
    }
  });

  return EXIT_SUCCESS;
}
