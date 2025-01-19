#include "shogle/shogle.hpp"

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

struct common_vertex {
  ntf::vec3 pos;
  ntf::vec3 normals;
  ntf::vec2 texcoords;

  static constexpr std::array<ntf::r_attrib_descriptor, 3> attribs {{
    { ntf::r_attrib_type::vec3, 0, 0, 0 },
    { ntf::r_attrib_type::vec3, 0, 1, sizeof(ntf::vec3) },
    { ntf::r_attrib_type::vec2, 0, 2, 2*sizeof(ntf::vec3) },
  }};
  static constexpr ntf::uint32 count = 3;
  static constexpr size_t stride = 2*sizeof(ntf::vec3)+sizeof(ntf::vec2);
  static constexpr ntf::r_vertex_attrib verex_attrib() {
    return {
      .attribs = attribs.data(),
      .count = count,
      .stride = stride,
    };
  }
};


int main() {
  ntf::logger::set_level(ntf::log_level::verbose);
  ntf::r_window window{ntf::r_window_params{
    .width = 1280,
    .height = 720,
    .title = "test - hello_cirno - ShOGLE " SHOGLE_VERSION_STRING,
  }};

  ntf::r_context ctx;
  ctx.init(window);
  if (!ctx) {
    ntf::logger::error("Failed to initialize context");
    return EXIT_FAILURE;
  }

  window.key_event([&](ntf::keycode code, auto, ntf::keystate state, auto) {
    if (code == ntf::keycode::key_escape && state == ntf::keystate::press) {
      window.close();
    }
  });
  window.viewport_event([&](ntf::uint32 w, ntf::uint32 h) {
    ctx.framebuffer_viewport(ntf::uvec4{0, 0, w, h});
  });

  auto load_tex = [&](std::string_view path) {
    ntf::texture_data image{path};
    NTF_ASSERT(image);
    auto tex = ctx.create_texture({
      .type = ntf::r_texture_type::texture2d,
      .format = ntf::r_texture_format::rgb8n,
      .extent = ntf::uvec3{image.dim(), 0},
      .layers = 1,
      .levels = 7,
      .gen_mipmaps = false,
      .sampler = ntf::r_texture_sampler::nearest,
      .addressing = ntf::r_texture_address::repeat,
    });
    NTF_ASSERT(tex);
    ctx.update(tex, image.data(), ntf::r_texture_format::rgb8n, ntf::uvec3{0, 0, 0}, 0, 0, true);
    image.unload();
    return tex;
  };
  auto load_buffer = [&](auto& data, ntf::r_buffer_type type) {
    auto buff = ctx.create_buffer({
      .type = type,
      .data = data,
      .size = sizeof(data),
    });
    NTF_ASSERT(buff);
    return buff;
  };
  auto load_shader = [&](std::string_view src, ntf::r_shader_type type) {
    auto shad = ctx.create_shader({
      .type = type,
      .source = src,
    });
    NTF_ASSERT(shad);
    return shad;
  };
  auto load_pipeline = [&](ntf::r_shader_handle vert, ntf::r_shader_handle frag) {
    ntf::r_shader_handle shads[] = {vert, frag};
    auto pipe = ctx.create_pipeline({
      .stages = shads,
      .stage_count = 2,
      .attribs = common_vertex::verex_attrib(),
      .primitive = ntf::r_primitive::triangles,
      .poly_mode = ntf::r_polygon_mode::fill,
      .front_face = ntf::r_front_face::clockwise,
      .cull_mode = ntf::r_cull_mode::front_back,
      .tests = ntf::r_pipeline_test::all,
      .depth_compare_op = ntf::r_compare_op::less,
      .stencil_compare_op = ntf::r_compare_op::less,
    });
    NTF_ASSERT(pipe);
    return pipe;
  };

  auto tex = load_tex("./examples/res/cirno_cpp.jpg");

  auto cube_vbo = load_buffer(ntf::cube_vertices, ntf::r_buffer_type::vertex);
  auto quad_vbo = load_buffer(ntf::quad_vertices, ntf::r_buffer_type::vertex);
  auto quad_ebo = load_buffer(ntf::quad_indices, ntf::r_buffer_type::index);

  auto vertex = load_shader(vert_src, ntf::r_shader_type::vertex);
  auto fragment_color = load_shader(frag_src, ntf::r_shader_type::fragment);
  auto fragment_tex = load_shader(frag_tex_src, ntf::r_shader_type::fragment);
  auto pipe_col = load_pipeline(vertex, fragment_color);
  auto pipe_tex = load_pipeline(vertex, fragment_tex);

  auto fb_tex = ctx.create_texture({
    .type = ntf::r_texture_type::texture2d,
    .format = ntf::r_texture_format::rgb8n,
    .extent = ntf::uvec3{1280, 720, 0},
    .layers = 1,
    .levels = 1,
    .gen_mipmaps = false,
    .sampler = ntf::r_texture_sampler::nearest,
    .addressing = ntf::r_texture_address::repeat,
  });
  NTF_ASSERT(fb_tex);
  ntf::r_framebuffer_att fb_att[] = {
    {fb_tex, 0, 0},
  };
  auto fbo = ctx.create_framebuffer({
    .extent = ntf::uvec2{1280, 720},
    .test_buffers = ntf::r_test_buffer_flag::both,
    .test_buffer_format = ntf::r_test_buffer_format::depth24u_stencil8u,
    .attachments = fb_att,
    .attachment_count = 1,
  });
  NTF_ASSERT(fbo);
  ctx.framebuffer_viewport(fbo, ntf::uvec4{0, 0, 1280, 720});

  auto u_col_model = ctx.query(pipe_col, ntf::r_query_uniform, "model");
  auto u_col_proj = ctx.query(pipe_col, ntf::r_query_uniform, "proj");
  auto u_col_view = ctx.query(pipe_col, ntf::r_query_uniform, "view");
  auto u_col_color = ctx.query(pipe_col, ntf::r_query_uniform, "color");

  auto u_tex_model = ctx.query(pipe_tex, ntf::r_query_uniform, "model");
  auto u_tex_proj = ctx.query(pipe_tex, ntf::r_query_uniform, "proj");
  auto u_tex_view = ctx.query(pipe_tex, ntf::r_query_uniform, "view");
  auto u_tex_color = ctx.query(pipe_tex, ntf::r_query_uniform, "color");
  auto u_tex_sampler = ctx.query(pipe_tex, ntf::r_query_uniform, "sampler0");

  ntf::mat4 cam_view_cube = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 0.f, -3.f});
  ntf::mat4 cam_proj_cube = glm::perspective(glm::radians(45.f), 1280.f/720.f, .1f, 100.f);
  auto transf_cube = ntf::transform3d{}.pos(0.f, 0.f, 0.f).scale(1.f);
  ntf::mat4 transf_mat_cube = transf_cube.mat();
  ntf::vec4 color_cube {0.f, 1.f, 0.f, 1.f};

  ntf::mat4 cam_view_quad = glm::translate(glm::mat4{1.f}, glm::vec3{640.f, 360.f, -3.f});
  ntf::mat4 cam_proj_quad = glm::ortho(0.f, 1280.f, 0.f, 720.f, .1f, 100.f);

  auto transf_quad0 = ntf::transform2d{}.pos(-200.f, 0.f).scale(ntf::vec2{300.f, -300.f});
  ntf::mat4 transf_mat_quad0 = transf_quad0.mat();

  ntf::float32 ratio = 1280.f/720.f;
  auto transf_quad1 = ntf::transform2d{}.pos(200.f, 0.f).scale(ntf::vec2{ratio*300.f, 300.f});
  ntf::mat4 transf_mat_quad1 = transf_quad1.mat();

  ntf::color4 main_color{.3f, .3f, .3f, 1.f};
  ntf::color4 fbo_color{1.f, 0.f, 0.f, 1.f};
  ctx.framebuffer_color(main_color);
  ctx.framebuffer_color(fbo, fbo_color);

  ntf::vec4 color_quad {1.f, 1.f, 1.f, 1.f};

  ntf::float32 t = 0;
  ntf::float64 avg_fps{0};
  ntf::float64 fps[120] = {0};
  ntf::uint8 fps_counter{0};
  ntf::float32 t2 = 0;

  ntf::shogle_render_loop(ctx, [&](ntf::float64 dt) {
    // Using an ode solver instead of t+=dt just because
    t = ntf::ode_euler<ntf::float32>{}(0.f, t, dt, [](...) { return glm::pi<ntf::float32>(); });
    t2 = ntf::ode_euler<ntf::float32>{}(0.f, t2, dt, [](...) { return 1.f; });

    transf_cube.rot(ntf::axisquat(t, ntf::vec3{0.f, 1.f, 0.f}));
    transf_mat_cube = transf_cube.mat();

    transf_quad0.rot(-t);
    transf_mat_quad0 = transf_quad0.mat();

    transf_quad1.rot(t);
    transf_mat_quad1 = transf_quad1.mat();

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

    ImGui::Begin("test");
      ImGui::Text("Delta time: %f", dt);
      ImGui::Text("Avg fps: %f", avg_fps);
    ImGui::End();
    ImGui::ShowDemoWindow();

    ctx.framebuffer_clear(ntf::r_clear_flag::color_depth);
    ctx.framebuffer_clear(fbo, ntf::r_clear_flag::color_depth);

    ctx.bind_framebuffer(ntf::r_context::DEFAULT_FRAMEBUFFER);

    ctx.bind_vertex_buffer(cube_vbo);
    ctx.bind_pipeline(pipe_col);
    ctx.push_uniform(u_col_model, transf_mat_cube);
    ctx.push_uniform(u_col_proj, cam_proj_cube);
    ctx.push_uniform(u_col_view, cam_view_cube);
    ctx.push_uniform(u_col_color, color_cube);
    ctx.draw_opts({
      .count = 36,
      .offset = 0,
      .instances = 0,
    });
    ctx.submit();

    ctx.bind_vertex_buffer(quad_vbo);
    ctx.bind_index_buffer(quad_ebo);
    ctx.bind_pipeline(pipe_tex);
    ctx.bind_texture(tex, 0);
    ctx.push_uniform(u_tex_model, transf_mat_quad0);
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

    ctx.bind_vertex_buffer(quad_vbo);
    ctx.bind_index_buffer(quad_ebo);
    ctx.bind_pipeline(pipe_tex);
    ctx.bind_texture(fb_tex, 0);
    ctx.push_uniform(u_tex_model, transf_mat_quad1);
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

    ctx.bind_framebuffer(fbo);

    ctx.bind_vertex_buffer(cube_vbo);
    ctx.bind_pipeline(pipe_col);
    ctx.push_uniform(u_col_model, transf_mat_cube);
    ctx.push_uniform(u_col_proj, cam_proj_cube);
    ctx.push_uniform(u_col_view, cam_view_cube);
    ctx.push_uniform(u_col_color, color_cube);
    ctx.draw_opts({
      .count = 36,
      .offset = 0,
      .instances = 0,
    });
    ctx.submit();

    ctx.bind_vertex_buffer(quad_vbo);
    ctx.bind_index_buffer(quad_ebo);
    ctx.bind_pipeline(pipe_tex);
    ctx.bind_texture(tex, 0);
    ctx.push_uniform(u_tex_model, transf_mat_quad0);
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
  });

  ctx.destroy(fbo);
  ctx.destroy(fb_tex);
  ctx.destroy(pipe_tex);
  ctx.destroy(pipe_col);
  ctx.destroy(fragment_tex);
  ctx.destroy(fragment_color);
  ctx.destroy(vertex);
  ctx.destroy(quad_ebo);
  ctx.destroy(quad_vbo);
  ctx.destroy(cube_vbo);
  ctx.destroy(tex);

  return EXIT_SUCCESS;
}
