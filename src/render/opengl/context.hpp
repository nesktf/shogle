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

#define SHOGLE_DEFINE_GL_RESOURCE(__type, __handle_type, __store) \
template<> \
class gl_resource<__type> {\
private: \
  gl_resource(gl_context& ctx, r_handle handle) : _ctx(&ctx), _handle(handle) {} \
public: \
  const __type& get() const { return _ctx->__store[_handle]; } \
  __type& get() { return _ctx->__store[_handle]; } \
  const __type* operator->() const { return &get(); } \
  __type* operator->() { return &get(); } \
  const __type& operator*() const { return get(); } \
  __type& operator*() { return get(); } \
  operator __handle_type() const { return {_handle, r_api::opengl}; } \
private: \
  gl_context* _ctx; \
  r_handle _handle; \
private: \
  friend class gl_context; \
}

namespace ntf {

template<typename RenderContext>
class glfw_window;

enum class gl_texture_err {
  none = 0,
};

enum class gl_buffer_err {
  none = 0,
};

enum class gl_pipeline_err {
  none = 0,
};

enum class gl_shader_err {
  none = 0,
};

enum class gl_framebuffer_err {
  none = 0,
};

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
  expected<gl_resource<gl_texture>, gl_texture_err> make_texture(r_texture_descriptor desc);
  expected<gl_resource<gl_buffer>, gl_buffer_err> make_buffer(r_buffer_descriptor desc);
  expected<gl_resource<gl_pipeline>, gl_pipeline_err> make_pipeline(r_pipeline_descriptor desc);
  expected<gl_resource<gl_shader>, gl_shader_err> make_shader(r_shader_descriptor desc);
  expected<gl_resource<gl_framebuffer>, gl_framebuffer_err> make_target(r_target_descriptor desc);

  void destroy_texture(gl_resource<gl_texture> texture);
  void destroy_buffer(gl_resource<gl_buffer> buffer);
  void destroy_pipeline(gl_resource<gl_pipeline> pipeline);
  void destroy_shader(gl_resource<gl_shader> shader);
  void destroy_target(gl_resource<gl_framebuffer> target);

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
  static constexpr r_api RENDER_API = r_api::opengl;

private:
  GLADloadproc _proc_fun{nullptr};

  std::vector<gl_texture> _textures;
  std::vector<gl_buffer> _buffers;
  std::vector<gl_shader> _shaders;
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

  template<typename T>
  friend class gl_resource;
};

SHOGLE_DEFINE_GL_RESOURCE(gl_buffer, r_buffer, _buffers);
SHOGLE_DEFINE_GL_RESOURCE(gl_texture, r_texture, _textures);
SHOGLE_DEFINE_GL_RESOURCE(gl_pipeline, r_pipeline, _pipelines);
SHOGLE_DEFINE_GL_RESOURCE(gl_shader, r_shader, _shaders);
SHOGLE_DEFINE_GL_RESOURCE(gl_framebuffer, r_target, _framebuffers);

} // namespace ntf

#undef SHOGLE_DEFINE_GL_RESOURCE
