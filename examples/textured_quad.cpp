#include <shogle/math/transform.hpp>
#include <shogle/render/data.hpp>
#include <shogle/render/opengl.hpp>
#include <shogle/render/window.hpp>

#include <chimatools/chimatools.hpp>

namespace {

using namespace shogle::numdefs;

constexpr char vert_src[] = R"glsl(
#version 440 core

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

constexpr char frag_src[] = R"glsl(
#version 440 core

layout (location = 0) in vec4 frag_color;
layout (location = 1) in vec2 frag_uvs;

layout (location = 0) out vec4 out_color;

uniform sampler2D u_tex;
  
void main() {
  out_color = frag_color*texture(u_tex, frag_uvs);
}
)glsl";

struct quad_layout {
public:
  static constexpr size_t attribute_count = 3u;

public:
  struct pct_vertex {
    shogle::vec3 pos;
    shogle::vec4 color;
    shogle::vec2 uvs;
  };

public:
  static shogle::vertex_attrib_array<attribute_count> attributes() noexcept {
    return std::to_array<shogle::vertex_attribute>({
      {
        .location = 0,
        .type = shogle::attribute_type::vec3,
        .offset = offsetof(pct_vertex, pos),
        .stride = sizeof(pct_vertex),
      },
      {
        .location = 1,
        .type = shogle::attribute_type::vec4,
        .offset = offsetof(pct_vertex, color),
        .stride = sizeof(pct_vertex),
      },
      {
        .location = 2,
        .type = shogle::attribute_type::vec2,
        .offset = offsetof(pct_vertex, uvs),
        .stride = sizeof(pct_vertex),
      },
    });
  };
};

// clang-format off
constexpr auto vertices = std::to_array<quad_layout::pct_vertex>({
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

  chima::context chima;
  chima::image cirno(chima, CHIMA_DEPTH_8U, RES_FOLDER "/cirno_cpp.jpg");
  chima::scoped_resource cirno_defer(chima, cirno);
  const auto [tex_w, tex_h] = cirno.extent();

  f32 win_w = 800;
  f32 win_h = 600;

  const auto glfw = shogle::glfw_win::initialize_lib();
  const auto hints = shogle::glfw_gl_hints::make_default(4, 4);
  shogle::glfw_win win((u32)win_w, (u32)win_h, "test", hints);
  shogle::gl_context gl(win.surface_provider());

  bool pause = false;
  win.set_key_input_callback([&](auto, const shogle::glfw_key_data& key) {
    if (key.key == GLFW_KEY_SPACE && key.action == GLFW_PRESS) {
      pause = !pause;
    }
  });
  win.set_viewport_callback([&](auto, const shogle::extent2d& vp) {
    win_w = (f32)vp.width;
    win_h = (f32)vp.height;
  });

  shogle::gl_buffer quad_vbo(gl, vbo_size);
  shogle::gl_defer vbo_defer(gl, quad_vbo);
  shogle::gl_buffer quad_ebo(gl, ebo_size);
  shogle::gl_defer ebo_defer(gl, quad_ebo);
  shogle::gl_layout_builder layout_builder;
  auto layout = layout_builder.set_vertex_buffer(quad_vbo)
                  .set_index_buffer(quad_ebo, shogle::gl_vertex_layout::INDEX_FORMAT_U16)
                  .build<quad_layout>(gl)
                  .value();
  const shogle::gl_defer layout_defer(gl, layout);
  layout.vertex_buffer()->upload_data(gl, vertices.data(), vbo_size, 0).value();
  layout.index_buffer()->upload_data(gl, indices.data(), ebo_size, 0).value();

  shogle::gl_shader vert_shader(gl, vert_src, sizeof(vert_src), shogle::gl_shader::STAGE_VERTEX);
  const shogle::gl_defer vert_shader_defer(gl, vert_shader);
  shogle::gl_shader frag_shader(gl, frag_src, sizeof(frag_src), shogle::gl_shader::STAGE_FRAGMENT);
  const shogle::gl_defer frag_shader_defer(gl, frag_shader);

  shogle::gl_pipeline_builder pipeline_builder;
  auto pipeline = pipeline_builder.set_primitive(shogle::gl_pipeline::PRIMITIVE_TRIANGLES)
                    .set_polygon_mode(shogle::gl_pipeline::POLY_MODE_FILL)
                    .set_polygon_width(1.f)
                    .add_shader(vert_shader)
                    .add_shader(frag_shader)
                    .build(gl)
                    .value();

  const shogle::gl_defer pipeline_defer(gl, pipeline);
  const auto u_model = pipeline.uniform_location(gl, "u_model").value();
  const auto u_proj = pipeline.uniform_location(gl, "u_proj").value();
  const auto u_tex = pipeline.uniform_location(gl, "u_tex").value();

  shogle::gl_clear_builder clear_builder;
  const auto frame_clear = clear_builder.set_clear_color(.3f, .3f, .3f, 1.f)
                             .set_clear_flag(shogle::gl_clear_opts::CLEAR_COLOR)
                             .build();

  shogle::gl_texture_builder tex_builder;
  auto tex = tex_builder.set_type(shogle::gl_texture::TEX_TYPE_2D)
               .set_format(shogle::gl_texture::TEX_FORMAT_RGB8)
               .set_extent(shogle::extent2d(tex_w, tex_h))
               .set_min_sampler(shogle::gl_texture::SAMPLER_MIN_NEAREST)
               .set_mag_sampler(shogle::gl_texture::SAMPLER_MAG_NEAREST)
               .build(gl)
               .value();
  shogle::gl_defer tex_defer(gl, tex);
  const shogle::gl_texture::image_data d{
    .data = cirno.data(),
    .extent = {tex_w, tex_h, 1},
    .format = shogle::gl_texture::PIXEL_FORMAT_RGB,
    .datatype = shogle::gl_texture::PIXEL_TYPE_U8,
    .alignment = shogle::gl_texture::ALIGN_4BYTES,
  };
  tex.upload_image(gl, d).value();
  tex.generate_mipmaps(gl);

  f32 t = 0.f;
  shogle::gl_cmd_builder cmd_builder;
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
    const auto cmd = cmd_builder.set_vertex_layout(layout)
                       .set_pipeline(pipeline)
                       .set_draw_count(indices.size())
                       .add_texture(tex, 0)
                       .add_uniform(proj, u_proj)
                       .add_uniform(model, u_model)
                       .add_uniform(0, u_tex)
                       .build();
    gl.submit_immediate_command(cmd);
    gl.end_frame();
  });

  return EXIT_SUCCESS;
}
