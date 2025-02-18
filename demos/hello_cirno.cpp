#include "shogle/render.hpp"
#include "shogle/assets.hpp"
#include "shogle/math.hpp"

constexpr std::string_view vert_src = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;
uniform mat4 view;

void main() {
  gl_Position = proj*view*model*vec4(att_coords, 1.0f);
  tex_coord = att_texcoords;
}
)glsl";

constexpr std::string_view frag_src = R"glsl(
#version 460 core

out vec4 frag_color;

uniform vec4 color;

void main() {
  vec4 out_color = color;

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}
)glsl";


constexpr std::string_view frag_tex_src = R"glsl(
#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform vec4 color;
uniform sampler2D sampler0;

void main()  {
  vec4 out_color = color * texture(sampler0, tex_coord);

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}

)glsl";


int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  ntf::r_window window{ntf::r_window_params{
    .width = 1280,
    .height = 720,
    .title = "test - hello_cirno - ShOGLE " SHOGLE_VERSION_STRING,
    .x11_class_name = "test",
    .x11_instance_name = "test",
  }};

  auto ctx = ntf::r_context::create(window);
  if (!ctx) {
    ntf::logger::error("Failed to initialize context: {}", ctx.error().what());
    return EXIT_FAILURE;
  }

  window.key_event([&](ntf::keycode code, auto, ntf::keystate state, auto) {
    if (code == ntf::keycode::key_escape && state == ntf::keystate::press) {
      window.close();
    }
  });
  window.viewport_event([&](ntf::uint32 w, ntf::uint32 h) {
    ctx.framebuffer_viewport(ntf::r_context::DEFAULT_FRAMEBUFFER, ntf::uvec4{0, 0, w, h});
  });

  const auto image_flag = ntf::image_load_flags::flip_vertically;
  auto cirno_img = ntf::load_image<ntf::uint8>("./demos/res/cirno_cpp.jpg", image_flag);
  if (!cirno_img) {
    ntf::logger::error("[main] Failed to load cirno image: {}", cirno_img.error().what());
    return EXIT_FAILURE;
  }

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

  ntf::color4 main_color{.3f, .3f, .3f, 1.f};
  ctx.framebuffer_color(ntf::r_context::DEFAULT_FRAMEBUFFER, main_color);
  ctx.framebuffer_clear(ntf::r_context::DEFAULT_FRAMEBUFFER, ntf::r_clear_flag::color_depth);

  auto cirno_img_data = cirno_img->descriptor();
  auto tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8n,
    .extent = {cirno_img->dim(), 0},
    .layers = 1,
    .levels = 7,
    .images = {cirno_img_data},
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

  ntf::r_image_data fumo_img_data {
    .texels = fumo_diffuse->data(),
    .format = ntf::r_texture_format::rgb8n,
    .extent = ntf::uvec3{fumo_diffuse->dim(), 0},
    .offset = ntf::uvec3{0, 0, 0},
    .layer = 0,
    .level = 0,
  };
  auto fumo_tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8n,
    .extent = ntf::uvec3{fumo_diffuse->dim(), 0},
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
    .source = vert_src,
  });
  auto fragment_color = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::fragment,
    .source = frag_src,
  });
  auto fragment_tex = ntf::r_shader::create(ntf::unchecked, ctx, {
    .type = ntf::r_shader_type::fragment,
    .source = frag_tex_src,
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

  auto fb_tex = ntf::r_texture::create(ntf::unchecked, ctx, {
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8n,
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

  ntf::float32 fb_ratio = 1280.f/720.f;
  auto transf_cube = ntf::transform3d<ntf::float32>{}
    .pos(0.f, 0.f, 0.f).scale(1.f);
  ntf::mat4 cam_view_cube = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -3.f});
  ntf::mat4 cam_proj_cube = glm::perspective(glm::radians(45.f), fb_ratio, .1f, 100.f);
  ntf::vec4 color_cube {0.f, 1.f, 0.f, 1.f};

  auto transf_quad0 = ntf::transform2d<ntf::float32>{}
    .pos(-350.f, 0.f).scale(ntf::vec2{300.f, 300.f});
  auto transf_quad1 = ntf::transform2d<ntf::float32>{}
    .pos(350.f, 0.f).scale(ntf::vec2{fb_ratio*300.f, 300.f});
  ntf::mat4 cam_view_quad = glm::translate(glm::mat4{1.f}, glm::vec3{640.f, 360.f, -3.f});
  ntf::mat4 cam_proj_quad = glm::ortho(0.f, 1280.f, 0.f, 720.f, .1f, 100.f);
  ntf::vec4 color_quad {1.f, 1.f, 1.f, 1.f};

  const ntf::float32 fumo_scale = 0.04f;
  auto transf_fumo = ntf::transform3d<ntf::float32>{}
    .pos(0.f, -.3f, 0.f).scale(fumo_scale).pivot_x(1.f);

  ntf::float32 t = 0;
  ntf::float64 avg_fps{0};
  ntf::float64 fps[120] = {0};
  ntf::uint8 fps_counter{0};
  ntf::float32 t2 = 0;
  ntf::uint32 ups = 60;

  ntf::shogle_render_loop(ctx, ups, ntf::overload{
    [&](ntf::uint32 ups) {
      ntf::float32 dt = 1/static_cast<ntf::float32>(ups);
      t = ntf::ode_euler<ntf::float32>{}(0.f, t, dt, [](...) { return glm::pi<ntf::float32>(); });

      transf_cube
        .rot(ntf::axisquat(t, ntf::vec3{0.f, 1.f, 0.f}))
        .scale(ntf::vec3{1.f, .5f + std::abs(std::sin(t)), 1.f})
        .offset_y(-0.5f);
      transf_quad0.roll(-t);
      transf_quad1.roll(t);
      transf_fumo
        .rot(ntf::axisquat(glm::pi<ntf::float32>()*.5f*t, ntf::vec3{0.f, 1.f, 0.f}))
        .scale(ntf::vec3{
          fumo_scale,
          fumo_scale*.5f+fumo_scale*.5f*std::abs(std::sin(glm::pi<ntf::float32>()*t)),
          fumo_scale
        });
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
      ImGui::Begin("test");
        ImGui::Text("Delta time: %f", dt);
        ImGui::Text("Avg fps: %f", avg_fps);
        ImGui::Text("FDelta time: %f", 1/static_cast<float>(ups));
        ImGui::SliderScalar("ups", ImGuiDataType_U32, &ups, &ups_min, &ups_max);
      ImGui::End();
      ImGui::ShowDemoWindow();

      ctx.bind_framebuffer(ntf::r_context::DEFAULT_FRAMEBUFFER);

      ctx.bind_vertex_buffer(fumo_vbo.handle());
      ctx.bind_index_buffer(fumo_ebo.handle());
      ctx.bind_pipeline(pipe_tex.handle());
      ctx.bind_texture(fumo_tex.handle(), 0);
      ctx.push_uniform(u_tex_model, transf_fumo.world());
      ctx.push_uniform(u_tex_proj, cam_proj_cube);
      ctx.push_uniform(u_tex_view, cam_view_cube);
      ctx.push_uniform(u_tex_color, color_quad);
      ctx.push_uniform(u_tex_sampler, 0);
      ctx.draw_opts({
        .count = static_cast<ntf::uint32>(fumo_mesh.indices.count),
        .offset = 0,
        .instances = 0,
      });
      ctx.submit();

      ctx.bind_vertex_buffer(quad_vbo.handle());
      ctx.bind_index_buffer(quad_ebo.handle());
      ctx.bind_pipeline(pipe_tex.handle());
      ctx.bind_texture(tex.handle(), 0);
      ctx.push_uniform(u_tex_model, transf_quad0.world());
      ctx.push_uniform(u_tex_proj, cam_proj_quad);
      ctx.push_uniform(u_tex_view, cam_view_quad);
      ctx.push_uniform(u_tex_color, color_quad);
      ctx.push_uniform(u_tex_sampler, 0);
      ctx.draw_opts({
        .count = 6,
        .offset = 0,
        .instances = 0,
      });
      ctx.submit();

      ctx.bind_vertex_buffer(quad_vbo.handle());
      ctx.bind_index_buffer(quad_ebo.handle());
      ctx.bind_pipeline(pipe_tex.handle());
      ctx.bind_texture(fb_tex.handle(), 0);
      ctx.push_uniform(u_tex_model, transf_quad1.world());
      ctx.push_uniform(u_tex_proj, cam_proj_quad);
      ctx.push_uniform(u_tex_view, cam_view_quad);
      ctx.push_uniform(u_tex_color, color_quad);
      ctx.push_uniform(u_tex_sampler, 0);
      ctx.draw_opts({
        .count = 6,
        .offset = 0,
        .instances = 0,
      });
      ctx.submit();

      ctx.bind_framebuffer(fbo.handle());

      ctx.bind_vertex_buffer(cube_vbo.handle());
      ctx.bind_pipeline(pipe_col.handle());
      ctx.push_uniform(u_col_model, transf_cube.world());
      ctx.push_uniform(u_col_proj, cam_proj_cube);
      ctx.push_uniform(u_col_view, cam_view_cube);
      ctx.push_uniform(u_col_color, color_cube);
      ctx.draw_opts({
        .count = 36,
        .offset = 0,
        .instances = 0,
      });
      ctx.submit();

      ctx.bind_vertex_buffer(quad_vbo.handle());
      ctx.bind_index_buffer(quad_ebo.handle());
      ctx.bind_pipeline(pipe_tex.handle());
      ctx.bind_texture(tex.handle(), 0);
      ctx.push_uniform(u_tex_model, transf_quad0.world());
      ctx.push_uniform(u_tex_proj, cam_proj_quad);
      ctx.push_uniform(u_tex_view, cam_view_quad);
      ctx.push_uniform(u_tex_color, color_quad);
      ctx.push_uniform(u_tex_sampler, 0);
      ctx.draw_opts({
        .count = 6,
        .offset = 0,
        .instances = 0,
      });
      ctx.submit();
    }
  });

  return EXIT_SUCCESS;
}
