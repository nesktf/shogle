#include <shogle/shogle.hpp>
#include "./boilerplate.hpp"
#include <ntfstl/logger.hpp>

int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  
  auto cousine = shogle::load_font_atlas<char>("./demos/res/CousineNerdFont-Regular.ttf");
  if (!cousine) {
    ntf::logger::error("[main] Failed to load font: {}", cousine.error().what());
    return EXIT_FAILURE;
  }

  auto cirno_img = shogle::load_image<ntf::uint8>("./demos/res/cirno_cpp.jpg");
  if (!cirno_img) {
    ntf::logger::error("[main] Failed to load cirno image: {}", cirno_img.error().what());
    return EXIT_FAILURE;
  }

  auto atlas = shogle::load_atlas("./demos/res/2hus.json", shogle::atlas_load_flags::flip_y);
  if (!atlas) {
    ntf::logger::error("[main] Failed to load atlas: {}", atlas.error().what());
    return EXIT_FAILURE;
  }
  auto atlas_img = shogle::load_image<ntf::uint8>(atlas->image_path);
  if (!atlas_img) {
    ntf::logger::error("[main] Failed to load atlas image: {}", atlas_img.error().what());
    return EXIT_FAILURE;
  }

  auto vert_src = shogle::file_contents("./demos/res/shaders/vert_base.vs.glsl").value();
  auto frag_col_src = shogle::file_contents("./demos/res/shaders/frag_color.fs.glsl").value();
  auto frag_tex_src = shogle::file_contents("./demos/res/shaders/frag_tex.fs.glsl").value();
  auto vert_atl_src = shogle::file_contents("./demos/res/shaders/vert_atlas.vs.glsl").value();

  const auto fumo_flag = shogle::model_load_flags::triangulate;
  auto fumo = shogle::load_model<shogle::pnt_vertex>("./demos/res/cirno_fumo/cirno_fumo.obj", fumo_flag);
  if (!fumo) {
    ntf::logger::error("[main] Failed to load fumo model: {}", fumo.error().what());
    return EXIT_FAILURE;
  }

  auto fumo_diffuse = shogle::load_image<ntf::uint8>(fumo->materials.paths[0]);
  if (!fumo_diffuse) {
    ntf::logger::error("[main] Failed to load fumo material: {}", fumo_diffuse.error().what());
    return EXIT_FAILURE;
  }

  const auto& fumo_mesh = fumo->meshes.data[0];
  const auto& fumo_verts = fumo->meshes.vertices;
  const auto& fumo_inds = fumo->meshes.indices;

  const char win_title[] = "test - hello_cirno - " SHOGLE_VERSION;
  auto [window, ctx] = shogle::make_gl_ctx(1280, 720, win_title).value();
  auto imgui = shogle::imgui_ctx::create(ctx.get(), window.get());

  auto quad = shogle::quad_mesh::create(ctx).value();
  auto cube = shogle::cube_mesh::create(ctx).value();

  shogle::text_buffer text_buffer;
  shogle::mat4 cam_proj_fnt = glm::ortho(0.f, 1280.f, 0.f, 720.f);
  auto sdf_rule = shogle::sdf_text_rule::create(ctx,
                                             shogle::color3{1.f, 0.f, 0.f}, 0.5f, 0.05f,
                                             shogle::color3{0.f, 0.f, 0.f},
                                             shogle::vec2{-0.005f, -0.005f},
                                             0.62f, 0.05f);
  if (!sdf_rule) {
    ntf::logger::error("[main] Failed to create text render rule: {}", sdf_rule.error().what());
    return EXIT_FAILURE;
  }

  auto frenderer = shogle::font_renderer::create(ctx, cam_proj_fnt, std::move(*cousine));
  if (!frenderer) {
    ntf::logger::error("[main] Failed to create font renderer: {}", frenderer.error().what());
    return EXIT_FAILURE;
  }

  auto tex = shogle::make_texture2d(ctx, *cirno_img,
                                  shogle::texture_sampler::nearest,
                                  shogle::texture_addressing::repeat).value();
  auto atlas_tex = shogle::make_texture2d(ctx, *atlas_img,
                                        shogle::texture_sampler::nearest,
                                        shogle::texture_addressing::repeat).value();
  auto fumo_tex = shogle::make_texture2d(ctx, *fumo_diffuse,
                                       shogle::texture_sampler::linear,
                                       shogle::texture_addressing::repeat).value();

  const shogle::buffer_data fumo_vbo_data {
    .data = fumo_mesh.vertex_data(fumo_verts),
    .size = fumo_mesh.vertices_size(),
    .offset = 0u,
  };
  auto fumo_vbo = shogle::vertex_buffer::create(ctx, {
    .flags = shogle::buffer_flag::dynamic_storage,
    .size = fumo_mesh.vertices_size(),
    .data = fumo_vbo_data,
  }).value();

  const shogle::buffer_data fumo_ebo_data {
    .data = fumo_mesh.index_data(fumo_inds),
    .size = fumo_mesh.indices_size(),
    .offset = 0u,
  };
  auto fumo_ebo = shogle::index_buffer::create(ctx, {
    .flags = shogle::buffer_flag::dynamic_storage,
    .size = fumo_mesh.indices_size(),
    .data = fumo_ebo_data,
  }).value();


  auto vertex = shogle::vertex_shader::create(ctx, {vert_src}).value();
  auto vertex_atlas = shogle::vertex_shader::create(ctx, {vert_atl_src}).value();
  auto fragment_color = shogle::fragment_shader::create(ctx, {frag_col_src}).value();
  auto fragment_tex = shogle::fragment_shader::create(ctx, {frag_tex_src}).value();

  auto pipe_col = shogle::make_pipeline<shogle::pnt_vertex>(vertex, fragment_color).value();
  auto pipe_tex = shogle::make_pipeline<shogle::pnt_vertex>(vertex, fragment_tex).value();
  auto pipe_atl = shogle::make_pipeline<shogle::pnt_vertex>(vertex_atlas, fragment_tex).value();

  auto [fbo, fbo_tex] = shogle::make_fbo(ctx, {1280, 720}, {1.f, 0.f, 0.f, 1.f},
                                        shogle::clear_flag::color_depth).value();

  auto u_col_model = pipe_col.uniform_location("model").value();
  auto u_col_proj = pipe_col.uniform_location("proj").value();
  auto u_col_view = pipe_col.uniform_location("view").value();
  auto u_col_color = pipe_col.uniform_location("color").value();

  auto u_tex_model = pipe_tex.uniform_location("model").value();
  auto u_tex_proj = pipe_tex.uniform_location("proj").value();
  auto u_tex_view = pipe_tex.uniform_location("view").value();
  auto u_tex_color = pipe_tex.uniform_location("color").value();
  auto u_tex_sampler = pipe_tex.uniform_location("sampler0").value();

  auto u_atl_model = pipe_atl.uniform_location("model").value();
  auto u_atl_proj = pipe_atl.uniform_location("proj").value();
  auto u_atl_view = pipe_atl.uniform_location("view").value();
  auto u_atl_offset = pipe_atl.uniform_location("offset").value();
  auto u_atl_color = pipe_atl.uniform_location("color").value();
  auto u_atl_sampler = pipe_atl.uniform_location("sampler0").value();

  ntf::float32 fb_ratio = 1280.f/720.f;
  auto transf_cube = shogle::transform3d<ntf::float32>{}
    .pos(0.f, 0.f, 0.f).scale(1.f);
  shogle::mat4 cam_view_cube = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -3.f});
  shogle::mat4 cam_proj_fumo = glm::perspective(glm::radians(45.f), fb_ratio, .1f, 100.f);
  shogle::vec4 color_cube {0.f, 1.f, 0.f, 1.f};

  auto transf_quad0 = shogle::transform2d<ntf::float32>{}
    .pos(-350.f, 0.f).scale(shogle::vec2{300.f, 300.f});
  auto transf_quad1 = shogle::transform2d<ntf::float32>{}
    .pos(350.f, 0.f).scale(shogle::vec2{fb_ratio*300.f, 300.f});
  shogle::mat4 cam_view_quad = glm::translate(glm::mat4{1.f}, glm::vec3{640.f, 360.f, -3.f});
  shogle::mat4 cam_proj_quad = glm::ortho(0.f, 1280.f, 0.f, 720.f, .1f, 100.f);
  auto cam_proj_cube = cam_proj_fumo;
  shogle::vec4 color_quad {1.f, 1.f, 1.f, 1.f};

  const ntf::float32 fumo_scale = 0.04f;
  auto transf_fumo = shogle::transform3d<ntf::float32>{}
    .pos(0.f, -.3f, 0.f).scale(fumo_scale);

  const ntf::float32 rin_scale = 300.f;
  const auto& rin_seq = atlas->sequence_at(*atlas->find_sequence("rin.dance"));
  auto rin_base_index = rin_seq.entries.index;
  auto rin_count = rin_seq.entries.count;

  const shogle::vec4* rin_uvs = &atlas->sprites[atlas->indices[rin_base_index]].offset;
  const auto rin_aspect = atlas->sprites[atlas->indices[rin_base_index]].aspect();
  // const ntf::uvec2* rin_dim = &atlas.sprites[rin_seq.entries.index].dim;

  auto transf_rin = shogle::transform2d<ntf::float32>{}
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

  window.set_key_press_callback([&](shogle::window& win, const shogle::win_key_data& key_data) {
    const float ts = 4.f;
    if (key_data.action == shogle::win_action::press) {
      if (key_data.key == shogle::win_key::escape) {
        win.close();
      }
      if (key_data.key == shogle::win_key::space) {
        do_things = !do_things;
      }

      if (key_data.key == shogle::win_key::left) {
        t -= ts/static_cast<float>(ups);
      } else if (key_data.key == shogle::win_key::right) {
        t += ts/static_cast<float>(ups);
      }
    }
  });

  const auto default_fbo = shogle::framebuffer::get_default(ctx);
  window.set_viewport_callback([&](auto&, const shogle::extent2d& extent) {
    ntf::uint32 w = extent.x;
    ntf::uint32 h = extent.y;
    default_fbo.viewport(shogle::uvec4{0, 0, w, h});
    fb_ratio = static_cast<ntf::float32>(w)/static_cast<ntf::float32>(h);
    cam_proj_fumo = glm::perspective(glm::radians(45.f), fb_ratio, .1f, 100.f);
    // cam_proj_quad = glm::ortho(0.f, (float)w, 0.f, (float)h, .1f, 100.f);
    // cam_proj_fnt = glm::ortho(0.f, (float)w, 0.f, (float)h);
  });

  shogle::render_loop(window, ctx, ups, ntf::overload{
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
        .rot(t, shogle::vec3{0.f, 1.f, 0.f})
        .scale_y(.5f + std::abs(std::sin(t)))
        .offset_y(-0.5f);
      transf_quad0.roll(-t);
      transf_quad1.roll(t);
      transf_fumo
        .rot(glm::pi<ntf::float32>()*.5f*t, shogle::vec3{0.f, 1.f, 0.f})
        .scale_y(fumo_scale*.5f+fumo_scale*.5f*std::abs(std::sin(glm::pi<ntf::float32>()*t)));
    },
    [&](ntf::float64 dt, ntf::float64) {
      // Using an ode solver instead of t+=dt just because
      t2 = shogle::ode_euler<ntf::float32>{}(0.f, t2, dt, [](...) { return 1.f; });

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
      const shogle::uniform_const fumo_unifs[] = {
        shogle::format_uniform_const(u_tex_model, transf_fumo.world()),
        shogle::format_uniform_const(u_tex_proj, cam_proj_fumo),
        shogle::format_uniform_const(u_tex_view, cam_view_cube),
        shogle::format_uniform_const(u_tex_color, color_quad),
        shogle::format_uniform_const(u_tex_sampler, 0),
      };
      const shogle::uniform_const cino_unifs[] = {
        shogle::format_uniform_const(u_tex_model, transf_quad0.world()),
        shogle::format_uniform_const(u_tex_proj, cam_proj_quad),
        shogle::format_uniform_const(u_tex_view, cam_view_quad),
        shogle::format_uniform_const(u_tex_color, color_quad),
        shogle::format_uniform_const(u_tex_sampler, 0),
      };
      const shogle::uniform_const fb_unifs[] = {
        shogle::format_uniform_const(u_tex_model, transf_quad1.world()),
        shogle::format_uniform_const(u_tex_proj, cam_proj_quad),
        shogle::format_uniform_const(u_tex_view, cam_view_quad),
        shogle::format_uniform_const(u_tex_color, color_quad),
        shogle::format_uniform_const(u_tex_sampler, 0),
      };
      const shogle::uniform_const rin_unifs[] = {
        shogle::format_uniform_const(u_atl_model, transf_rin.world()),
        shogle::format_uniform_const(u_atl_proj, cam_proj_quad),
        shogle::format_uniform_const(u_atl_view, cam_view_quad),
        shogle::format_uniform_const(u_atl_color, color_quad),
        shogle::format_uniform_const(u_atl_offset, *rin_uvs),
        shogle::format_uniform_const(u_atl_sampler, 0),
      };
      const shogle::uniform_const cube_unifs[] = {
        shogle::format_uniform_const(u_col_model, transf_cube.world()),
        shogle::format_uniform_const(u_col_proj, cam_proj_cube),
        shogle::format_uniform_const(u_col_view, cam_view_cube),
        shogle::format_uniform_const(u_col_color, color_cube),
      };

      const shogle::render_opts fumo_opts {
        .vertex_count = static_cast<ntf::uint32>(fumo_mesh.indices.count),
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      };
      const shogle::render_opts quad_opts {
        .vertex_count = 6,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      };
      const shogle::render_opts cube_opts {
        .vertex_count = 36,
        .vertex_offset = 0,
        .index_offset = 0,
        .instances = 0,
      };

      const shogle::vertex_binding fumo_vertex_bind[] {
        {.buffer = fumo_vbo, .layout = 0u},
        {.buffer = fumo_vbo, .layout = 1u},
        {.buffer = fumo_vbo, .layout = 2u},
      };

      // Fumo
      const shogle::texture_binding fumo_tex_bind{.texture = fumo_tex, .sampler = 0};
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
      const shogle::texture_binding rin_tex_bind{.texture = atlas_tex, .sampler = 0};
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
      const shogle::texture_binding fbo_tex_bind{.texture = fbo_tex, .sampler = 0};
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
      const shogle::texture_binding cino_tex_bind{.texture = tex, .sampler = 0};
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
