#pragma once

// #include "./texture.hpp"
// #include "./shader.hpp"
// #include "./mesh.hpp"
// #include "./framebuffer.hpp"
// #include "./font.hpp"

#include "buffer.hpp"
#include "texture.hpp"
#include <queue>

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui_impl_opengl3.h>
#endif

#include "stl/expected.hpp"

#include <optional>
#include <vector>

namespace ntf {

template<typename RenderContext>
class glfw_window;

struct r_draw_cmd {

};

enum class r_texture_error {
  none = 0,
};

enum class r_buffer_error {
  none = 0,
};

enum class r_pipeline_error {
  none = 0,
};

enum class r_shader_error {
  none = 0,
};

class gl_context {
public:
  using texture_handle = r_texture<gl_context>;
  using buffer_handle = r_buffer<gl_context>;
  using pipeline_handle = r_pipeline<gl_context>;
  using shader_handle = r_shader<gl_context>;

  enum class init_err {
    none = 0,
  };

private:
  gl_context() = default;

private:
  init_err init(GLADloadproc proc);
  void destroy();

public:
  void enqueue(r_draw_cmd cmd);
  void draw_frame();

public:
  ntf::expected<texture_handle, r_texture_error> create_texture(r_texture_info info);
  void destroy_texture(texture_handle& texture);

  ntf::expected<buffer_handle, r_buffer_error> create_buffer(r_buffer_info info);
  void destroy_buffer(buffer_handle& buffer);

  ntf::expected<pipeline_handle, r_pipeline_error> create_pipeline(r_pipeline_info info);
  void destroy_pipeline(pipeline_handle& pipeline);

  ntf::expected<shader_handle, r_shader_error> create_shader(r_shader_info info);
  void destroy_shader(shader_handle& shader);

public:
  void viewport(uint32 x, uint32 y, uint32 w, uint32 h);
  void viewport(uvec2 pos, uvec2 sz);
  void viewport(uint32 w, uint32 h);
  void viewport(uvec2 sz);

  void clear_color(color4 color);

public:
  uvec4 viewport() const { return _viewport; }
  color4 clear_color() const { return _clear_color; }

  // const char* name() const { return (const char*)glGetString(GL_RENDERER); }
  // const char* vendor() const { return (const char*)glGetString(GL_VENDOR); }
  // const char* version() const{ return (const char*)glGetString(GL_VERSION); }

private:
  static GLAPIENTRY void _debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                         GLsizei len, const GLchar* msg, const void* user_ptr);

public:
  static constexpr r_backend backend = r_backend::opengl;

private:
  GLADloadproc _proc_fun{nullptr};
  uvec4 _viewport{0, 0, 0, 0};
  color4 _clear_color{.3f, .3f, .3f, 1.f};

  std::vector<gl_texture> _textures;
  std::vector<gl_buffer> _buffers;

  std::queue<r_draw_cmd> _cmds;

  struct {
    GLuint vao{0};
    GLuint vbo{0}, ebo{0}, ubo{0};
    GLuint program{0};
    GLuint texture{0};
  } _state;

public:
  NTF_DISABLE_MOVE_COPY(gl_context);

private:
  friend class glfw_window<gl_context>;
};

} // namespace ntf
