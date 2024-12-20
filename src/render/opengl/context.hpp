#pragma once

// #include "./texture.hpp"
// #include "./shader.hpp"
// #include "./mesh.hpp"
// #include "./framebuffer.hpp"
// #include "./font.hpp"

#include "buffer.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "framebuffer.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui_impl_opengl3.h>
#endif

#include "stl/expected.hpp"

#include <vector>
#include <queue>

namespace ntf {

template<typename RenderContext>
class glfw_window;

class gl_context {
public:
  gl_context() = default;

private:
  void init(GLADloadproc proc);
  void destroy();

public:
  void enqueue(r_draw_cmd cmd);
  void draw_frame();

public:
  ntf::expected<r_texture<gl_context>, r_texture_err> create_texture(r_texture_info texture);
  void destroy_texture(r_texture<gl_context>& texture);

  ntf::expected<r_buffer<gl_context>, r_buffer_err> create_buffer(r_buffer_info buffer);
  void destroy_buffer(r_buffer<gl_context>& buffer);

  ntf::expected<r_pipeline<gl_context>, r_pipeline_err> create_pipeline(r_pipeline_info pipeline);
  void destroy_pipeline(r_pipeline<gl_context>& pipeline);

  ntf::expected<r_shader<gl_context>, r_shader_err> create_shader(r_shader_info shader);
  void destroy_shader(r_shader<gl_context>& shader);

public:
  void viewport(uint32 x, uint32 y, uint32 w, uint32 h);
  void viewport(uint32 w, uint32 h);
  void viewport(uvec2 pos, uvec2 size);
  void viewport(uvec2 size);

  void clear_color(float32 r, float32 g, float32 b, float32 a);
  void clear_color(float32 r, float32 g, float32 b);
  void clear_color(color4 color);
  void clear_color(color3 color);

  void clear_flags(r_clear clear);

public:
  uvec4 viewport() const { return _glstate.viewport; }
  uvec2 viewport_pos() const { return uvec2{_glstate.viewport.x, _glstate.viewport.y}; }
  uvec2 viewport_size() const { return uvec2{_glstate.viewport.z, _glstate.viewport.w}; }

  color4 clear_color() const { return _glstate.clear_color; }
  r_clear clear_flags() const { return _glstate.clear_flags; }

  bool valid() const { return _proc_fun != nullptr; }
  explicit operator bool() const { return valid(); }

  std::string_view name() const;
  std::string_view vendor() const;
  std::string_view version() const;

private:
  static GLAPIENTRY void _debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                         GLsizei len, const GLchar* msg, const void* user_ptr);

public:
  static constexpr r_api render_api = r_api::opengl;

private:
  GLADloadproc _proc_fun{nullptr};

  std::vector<gl_texture> _textures;
  std::vector<gl_buffer> _buffers;
  std::vector<gl_shader> _shaders;
  std::vector<std::pair<std::vector<r_attrib_info>, size_t>> _attribs;
  std::vector<gl_pipeline> _pipelines;
  std::vector<gl_framebuffer> _framebuffers;

  std::queue<r_draw_cmd> _cmds;

  struct {
    GLuint vao{0};
    GLuint vbo{0}, ebo{0}, ubo{0};
    GLuint fbo{0};
    GLuint program{0};
    uint32 enabled_tex{0};

    uvec4 viewport{0, 0, 0, 0};
    color4 clear_color{.3f, .3f, .3f, 1.f};
    r_clear clear_flags{r_clear::none};
  } _glstate;

public:
  NTF_DISABLE_MOVE_COPY(gl_context);

private:
  friend class glfw_window<gl_context>;
};

// template<typename F>
// void gl_framebuffer::bind(F&& func) {
//   NTF_ASSERT(_fbo);
//
//   uvec4 main_vp = _ctx.viewport();
//   glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
//   glViewport(_viewport.x, _viewport.y, _viewport.z, _viewport.w);
//
//   gl_clear_bits(_clear, _clear_color);
//   func();
//
//   glBindFramebuffer(GL_FRAMEBUFFER, 0);
//   glViewport(main_vp.x, main_vp.y, main_vp.z, main_vp.w);
// }

template<>
class r_texture<gl_context> {

};

template<>
class r_buffer<gl_context> {

};

template<>
class r_pipeline<gl_context> {

};

template<>
class r_shader<gl_context> {

};

} // namespace ntf
