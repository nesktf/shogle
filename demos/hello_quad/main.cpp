#include <shogle/render/data.hpp>
#include <shogle/render/opengl.hpp>
#include <shogle/render/window.hpp>

namespace {

using namespace ntf::numdefs;

constexpr std::string_view vert_src = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_pos;
layout (location = 1) in vec4 att_color;

layout (location = 0) out vec4 frag_color;
  
void main() {
  gl_Position = vec4(att_pos, 1.0f);
  frag_color = att_color;
}  
)glsl";

constexpr std::string_view frag_src = R"glsl(
#version 460 core

layout (location = 0) in vec4 frag_color;

layout (location = 0) out vec4 out_color;
  
void main() {
  out_color = frag_color;
}
)glsl";

// clang-format off
constexpr auto vertices = std::to_array<shogle::pc_vertex>({
  // pos               // color
  {{-.5f, -.5f,  0.f}, {1.f, 0.f, 0.f, 1.f}},
  {{ .5f, -.5f,  0.f}, {0.f, 1.f, 0.f, 1.f}},
  {{ .5f,  .5f,  0.f}, {0.f, 0.f, 1.f, 1.f}},
  {{-.5f,  .5f,  0.f}, {1.f, 1.f, 1.f, 1.f}},
});
constexpr size_t vbo_size = vertices.size() * sizeof(vertices[0]);

constexpr auto indices = std::to_array<u16>({
  0, 1, 2,
	2, 3, 0,
});
constexpr size_t ebo_size = indices.size() * sizeof(indices[0]);
// clang-format on

} // namespace

int main() {
  ntf::logger::set_level(ntf::log_level::verbose);

  const auto glfw = shogle::glfw_win::initialize_lib();
  const auto hints = shogle::glfw_gl_hints::make_default(4, 6);
  shogle::glfw_win win(800, 600, "test", hints);
  shogle::gl_context gl(win);

  shogle::gl_vertex_layout quad_layout(gl, shogle::aos_vertex_arg<shogle::pc_vertex>{});
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

  shogle::gl_command_builder cmd_builder;
  const auto cmd =
    cmd_builder.set_vertex_layout(quad_layout)
      .set_pipeline(pipeline)
      .set_index_buffer(quad_ebo, shogle::gl_draw_command::INDEX_FORMAT_U16, (u32)indices.size())
      .add_vertex_buffer(0, quad_vbo)
      .set_viewport(0, 0, 800, 600)
      .set_instances(1)
      .build();

  shogle::gl_clear_builder frame_builder;
  const auto frame_clear = frame_builder.set_clear_color(.3f, .3f, .3f, 1.f)
                             .set_clear_flag(shogle::gl_clear_opts::CLEAR_COLOR)
                             .build();

  while (!win.should_close()) {
    win.poll_events();
    if (win.poll_key(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      win.close();
    }

    gl.start_frame(frame_clear);
    gl.submit_command(cmd);
    gl.end_frame();

    win.swap_buffers();
  }
  return EXIT_SUCCESS;
}
