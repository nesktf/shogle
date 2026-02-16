#include <shogle/math/transform.hpp>
#include <shogle/render/data.hpp>
#include <shogle/render/opengl.hpp>
#include <shogle/render/window.hpp>

#include <chimatools/chimatools.hpp>

namespace {

using namespace shogle::numdefs;

constexpr std::string_view vert_src = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_pos;
layout (location = 1) in vec4 att_color;
layout (location = 2) in vec2 att_uvs;

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec2 frag_uvs;

uniform mat4 u_proj;
uniform mat4 u_model;

void main() {
  gl_Position = u_proj*u_model*vec4(att_pos, 1.0f);
  frag_color = att_color;
  frag_uvs = att_uvs;
}  
)glsl";

constexpr std::string_view frag_src = R"glsl(
#version 460 core

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec2 frag_uvs;

layout (location = 0) out vec4 out_color;

uniform sampler2D u_tex;
  
void main() {
  out_color = frag_color*texture(u_tex, frag_uvs);
}
)glsl";

struct pct_vertex {
public:
  static constexpr u32 attribute_count = 3u;
  static constexpr inline auto attributes() noexcept;

public:
  shogle::vec3 pos;
  shogle::vec4 color;
  shogle::vec2 uvs;
};

constexpr inline auto pct_vertex::attributes() noexcept {
  return std::to_array<shogle::vertex_attribute>({
    {.location = 0, .type = shogle::attribute_type::vec3, .offset = offsetof(pct_vertex, pos)},
    {.location = 1, .type = shogle::attribute_type::vec4, .offset = offsetof(pct_vertex, color)},
    {.location = 2, .type = shogle::attribute_type::vec2, .offset = offsetof(pct_vertex, uvs)},
  });
}

static_assert(shogle::meta::vertex_type<pct_vertex>);

// clang-format off
constexpr auto vertices = std::to_array<pct_vertex>({
  // pos               // color              // tex
  {{-.5f, -.5f,  0.f}, {1.f, 0.f, 0.f, 1.f}, {0.f, 1.f}},
  {{ .5f, -.5f,  0.f}, {0.f, 1.f, 0.f, 1.f}, {1.f, 1.f}},
  {{ .5f,  .5f,  0.f}, {0.f, 0.f, 1.f, 1.f}, {1.f, 0.f}},
  {{-.5f,  .5f,  0.f}, {1.f, 1.f, 1.f, 1.f}, {0.f, 0.f}},
});
constexpr size_t vbo_size = vertices.size() * sizeof(vertices[0]);

constexpr auto indices = std::to_array<u16>({
  0, 1, 2, // bottom right triangle
  2, 3, 0, // top left triangle
});
constexpr size_t ebo_size = indices.size() * sizeof(indices[0]);
// clang-format on

} // namespace

int main() {
  shogle::logger::set_level(shogle::logger::LEVEL_VERBOSE);

  const auto glfw = shogle::glfw_win::initialize_lib();
  const auto hints = shogle::glfw_gl_hints::make_default(4, 6);
  shogle::glfw_win win(800, 600, "test", hints);
  shogle::gl_context gl(win);

  shogle::gl_vertex_layout quad_layout(gl, shogle::aos_vertex_arg<pct_vertex>{});
  const shogle::gl_scoped_resource layout_scope(gl, quad_layout);

  shogle::gl_buffer quad_vbo(gl, shogle::gl_buffer::TYPE_VERTEX, vbo_size);
  const shogle::gl_scoped_resource vbo_scope(gl, quad_vbo);
  quad_vbo.upload_data(gl, vertices.data(), vbo_size, 0).value();

  shogle::gl_buffer quad_ebo(gl, shogle::gl_buffer::TYPE_INDEX, ebo_size);
  const shogle::gl_scoped_resource ebo_scope(gl, quad_ebo);
  quad_ebo.upload_data(gl, indices.data(), ebo_size, 0).value();

  shogle::gl_shader vertex_shader(gl, vert_src, shogle::gl_shader::STAGE_VERTEX);
  const shogle::gl_scoped_resource vshader_scope(gl, vertex_shader);
  shogle::gl_shader fragment_shader(gl, frag_src, shogle::gl_shader::STAGE_FRAGMENT);
  const shogle::gl_scoped_resource fshader_scope(gl, fragment_shader);

  shogle::gl_shader_builder shader_builder;
  const auto pipeline_shaders =
    shader_builder.add_shader(vertex_shader).add_shader(fragment_shader).build();

  shogle::gl_graphics_pipeline pipeline(gl, pipeline_shaders);
  const shogle::gl_scoped_resource pipeline_scope(gl, pipeline);
  const auto u_model = pipeline.uniform_location(gl, "u_model").value();
  const auto u_proj = pipeline.uniform_location(gl, "u_proj").value();
  const auto u_tex = pipeline.uniform_location(gl, "u_tex").value();

  shogle::gl_clear_builder clear_builder;
  const auto frame_clear = clear_builder.set_clear_color(.3f, .3f, .3f, 1.f)
                             .set_clear_flag(shogle::gl_clear_opts::CLEAR_COLOR)
                             .build();

  chima::context chima;
  chima::image cirno(chima, CHIMA_DEPTH_8U, "./demos/res/cirno_cpp.jpg");
  chima::scoped_resource cirno_scope(chima, cirno);
  const auto [w, h] = cirno.extent();

  shogle::gl_texture tex(gl, shogle::gl_texture::TEX_FORMAT_RGB8, shogle::extent2d{w, h});
  shogle::gl_scoped_resource tex_scope(gl, tex);
  const shogle::gl_texture::image_data d{
    .data = cirno.data(),
    .extent = {w, h, 1},
    .format = shogle::gl_texture::PIXEL_FORMAT_RGB,
    .datatype = shogle::gl_texture::PIXEL_TYPE_U8,
    .alignment = shogle::gl_texture::ALIGN_4BYTES,
  };
  tex.upload_image(gl, d).value();
  tex.generate_mipmaps(gl);

  shogle::gl_command_builder cmd_builder;
  f32 t = 0.f;
  bool pause = false;
  win.set_key_input_callback([&](auto, const shogle::glfw_key_data& key) {
    if (key.key == GLFW_KEY_SPACE && key.action == GLFW_PRESS) {
      pause = !pause;
    }
  });
  f32 win_w = 800;
  f32 win_h = 600;
  win.set_viewport_callback([&](auto, const shogle::extent2d& vp) {
    win_w = (f32)vp.width;
    win_h = (f32)vp.height;
  });

  shogle::render_loop(win, [&](f64 dt) {
    if (win.poll_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      win.close();
    }

    gl.start_frame(frame_clear);
    if (!pause) {
      t += (f32)dt;
    }
    const f32 rot = t * shogle::math::pi<f32>;
    const auto proj = shogle::math::ortho(0.f, win_w, 0.f, win_h);
    auto model =
      shogle::math::translate(shogle::mat4(1.f), shogle::vec3(win_w / 2.f, win_h / 2.f, 1.f));
    model = shogle::math::rotate(model, rot, shogle::vec3(0.f, 0.f, 1.f));
    model = shogle::math::scale(model, shogle::vec3(400.f, 400.f, 1.f));

    cmd_builder.reset();
    const auto cmd = cmd_builder.set_vertex_layout(quad_layout)
                       .set_pipeline(pipeline)
                       .set_index_buffer(quad_ebo, shogle::gl_draw_command::INDEX_FORMAT_U16)
                       .set_draw_count(indices.size())
                       .add_texture(tex, 0)
                       .add_uniform(proj, u_proj)
                       .add_uniform(model, u_model)
                       .add_uniform(0, u_tex)
                       .add_vertex_buffer(quad_vbo)
                       .build();
    gl.submit_command(cmd);
    gl.end_frame();
  });

  return EXIT_SUCCESS;
}
