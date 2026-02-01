#include <shogle/render/data.hpp>
#include <shogle/render/opengl.hpp>

#include <GLFW/glfw3.h>

namespace {

using namespace ntf::numdefs;

class glfw_surface : public shogle::gl_surface_provider {
public:
  glfw_surface(GLFWwindow* win) : _win(win) {}

public:
  shogle::PFN_glGetProcAddress proc_loader() noexcept override {
    return reinterpret_cast<shogle::PFN_glGetProcAddress>(glfwGetProcAddress);
  }

  shogle::extent2d surface_extent() const noexcept override {
    int w, h;
    glfwGetFramebufferSize(_win, &w, &h);
    return {.width = (u32)w, .height = (u32)h};
  }

  void swap_buffers() noexcept override { glfwSwapBuffers(_win); }

private:
  GLFWwindow* _win;
};

GLFWwindow* init_window(u32 width, u32 height) {
  if (!glfwInit()) {
    return nullptr;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* win = glfwCreateWindow(width, height, "test", nullptr, nullptr);
  if (!win) {
    glfwTerminate();
    return win;
  }
  glfwMakeContextCurrent(win);
  return win;
}

void destroy_window(GLFWwindow* win) {
  glfwDestroyWindow(win);
  glfwTerminate();
}

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

constexpr auto indices = std::to_array<u16>({
  0, 1, 2,
	2, 3, 0,
});
// clang-format on

void run_demo(GLFWwindow* win, shogle::gl_context& gl) {
  shogle::gl_vertex_layout quad_layout(gl, shogle::aos_vertex_arg<shogle::pc_vertex>{});
  const shogle::scope_end layout_end = [&]() {
    shogle::gl_vertex_layout::destroy(gl, quad_layout);
  };

  static constexpr size_t vbo_size = vertices.size() * sizeof(vertices[0]);
  shogle::gl_buffer quad_vbo(gl, shogle::gl_buffer::BUFFER_VERTEX, vbo_size);
  const shogle::scope_end vbo_end = [&]() {
    shogle::gl_buffer::deallocate(gl, quad_vbo);
  };
  quad_vbo.upload_data(gl, vertices.data(), vbo_size, 0).value();

  static constexpr size_t ebo_size = indices.size() * sizeof(indices[0]);
  shogle::gl_buffer quad_ebo(gl, shogle::gl_buffer::BUFFER_INDEX, ebo_size);
  const shogle::scope_end ebo_end = [&]() {
    shogle::gl_buffer::deallocate(gl, quad_ebo);
  };
  quad_ebo.upload_data(gl, indices.data(), ebo_size, 0).value();

  shogle::gl_shader vertex_shader(gl, vert_src, shogle::gl_shader::STAGE_VERTEX);
  const shogle::scope_end vertex_shader_end = [&]() {
    shogle::gl_shader::destroy(gl, vertex_shader);
  };

  shogle::gl_shader fragment_shader(gl, frag_src, shogle::gl_shader::STAGE_FRAGMENT);
  const shogle::scope_end fragment_shader_end = [&]() {
    shogle::gl_shader::destroy(gl, fragment_shader);
  };

  const auto shader_set = shogle::gl_shader_builder{}
                            .add_shader(vertex_shader)
                            .add_shader(fragment_shader)
                            .build()
                            .value();

  shogle::gl_graphics_pipeline pipeline(gl, shader_set,
                                        shogle::gl_graphics_pipeline::PRIMITIVE_TRIANGLES,
                                        shogle::gl_graphics_pipeline::POLY_MODE_FILL);
  const shogle::scope_end pipeline_end = [&]() {
    shogle::gl_graphics_pipeline::destroy(gl, pipeline);
  };

  const shogle::gl_buffer_binding vertex_bind{
    .buffer = quad_vbo,
    .location = 0,
  };
  const shogle::gl_indexed_cmd cmd{
    .vertex_layout = quad_layout,
    .pipeline = pipeline,
    .index_buffer = quad_ebo,
    .vertex_buffers = {vertex_bind},
    .shader_buffers = {},
    .textures = {},
    .uniforms = {},
    .viewport = {0, 0, 800, 600},
    .scissor = {0, 0, 800, 600},
    .vertex_offset = 0,
    .vertex_count = 6,
    .index_count = static_cast<u32>(indices.size()),
    .format = shogle::gl_indexed_cmd::INDEX_FORMAT_U32,
    .instances = 1,
  };

  const shogle::gl_frame_initializer frame_init{
    .clear_opt =
      {
        .color = {.3f, .3f, .3f, 1.f},
        .clear_flags = shogle::gl_clear_opt::CLEAR_COLOR,
      },
    .fbos = {},
  };

  while (!glfwWindowShouldClose(win)) {
    glfwPollEvents();
    if (glfwGetKey(win, GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(win, 1);
    }

    gl.start_frame(frame_init);
    gl.submit_indexed_draw_command(cmd);
    gl.end_frame(); // implicitly swaps buffers
  }
}

} // namespace

int main() {
  auto win = init_window(800, 600);
  if (!win) {
    const char* err;
    auto code = glfwGetError(&err);
    ntf::logger::error("Failed to initialize GLFW window: ({}) {}", code, code ? err : "");
    return EXIT_FAILURE;
  }
  const shogle::scope_end clean_win = [&]() {
    destroy_window(win);
  };

  glfw_surface glfw_gl(win);
  auto ctx = shogle::gl_context::create(glfw_gl);
  if (!ctx) {
    ntf::logger::error("Failed to create OpenGL context: {}", ctx.error());
    return EXIT_FAILURE;
  }
  // gl_context's destructor frees the OpenGL context

  run_demo(win, *ctx);
  return EXIT_SUCCESS;
}
