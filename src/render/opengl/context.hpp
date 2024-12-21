#pragma once

#include "opengl.hpp"
#include "buffer.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "pipeline.hpp"
#include "framebuffer.hpp"

#include "stl/function.hpp"
#include "stl/expected.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x
#endif

#include <vector>
#include <queue>

namespace ntf {

// TODO: Define proper errors lol
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
private:
  template<typename T>
  class res_container {
  public:
    res_container() = default;

  public:
    void clear() {
      for (auto& res : _res) {
        if (res.complete()) {
          res.unload();
        }
      }
      _res.clear();
      _free = {};
    }

    r_handle acquire(gl_context& ctx) {
      if (!_free.empty()) {
        r_handle pos = _free.front();
        _free.pop();
        return pos;
      }
      _res.emplace_back(T{ctx});
      return _res.size()-1;
    }

    void push(r_handle pos) {
      NTF_ASSERT(pos < _res.size());
      _free.push(pos);
    }

    T& get(r_handle pos) {
      NTF_ASSERT(pos < _res.size());
      return _res[pos];
    }

    const T& get(r_handle pos) const {
      NTF_ASSERT(pos < _res.size());
      return _res[pos];
    }

  private:
    std::vector<T> _res;
    std::queue<r_handle> _free;
  };

public:
  gl_context(uint32 major, uint32 minor) :
    _major(major), _minor(minor) {}

private:
  template<typename Proc>
  void init(Proc proc, ntf::inplace_function<void()> swap_buffers_fun) {
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(proc))) {
      return;
    }
    _proc_fun = reinterpret_cast<GLADloadproc>(proc);
    _swap_buffers = std::move(swap_buffers_fun);
    _init_state();
  }
  void destroy();

private:
  void _init_state();

public:
  void start_frame();
  void enqueue(r_draw_cmd cmd);
  void end_frame();
  void device_wait() {}

public:
  expected<r_texture, gl_texture_err> make_texture(r_texture_descriptor desc);
  void destroy(r_texture texture);
  const gl_texture& resource(r_texture texture) const;
  gl_texture& resource(r_texture texture);

  expected<r_buffer, gl_buffer_err> make_buffer(r_buffer_descriptor desc);
  void destroy(r_buffer buffer);
  const gl_buffer& resource(r_buffer buffer) const;
  gl_buffer& resource(r_buffer buffer);

  expected<r_pipeline, gl_pipeline_err> make_pipeline(r_pipeline_descriptor desc);
  void destroy(r_pipeline pipeline);
  const gl_pipeline& resource(r_pipeline pipeline) const;
  gl_pipeline& resource(r_pipeline pipeline);

  expected<r_shader, gl_shader_err> make_shader(r_shader_descriptor desc);
  void destroy(r_shader shader);
  const gl_shader& resource(r_shader shader) const;
  gl_shader& resource(r_shader shader);

  expected<r_framebuffer, gl_framebuffer_err> make_framebuffer(r_framebuffer_descriptor desc);
  void destroy(r_framebuffer framebuffer);
  const gl_framebuffer& resource(r_framebuffer framebuffer) const;
  gl_framebuffer& resource(r_framebuffer framebuffer);

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

  std::string_view name_str() const;
  std::string_view vendor_str() const;
  std::string_view version_str() const;
  std::pair<uint32, uint32> version() const { return std::make_pair(_major, _minor); }

private:
  static GLAPIENTRY void _debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity,
                                         GLsizei len, const GLchar* msg, const void* user_ptr);

public:
  static constexpr r_api RENDER_API = r_api::opengl;

private:
  uint32 _major, _minor; // Core only
  GLADloadproc _proc_fun{nullptr};
  ntf::inplace_function<void()> _swap_buffers;

  res_container<gl_texture> _textures;
  res_container<gl_buffer> _buffers;
  res_container<gl_pipeline> _pipelines;
  res_container<gl_shader> _shaders;
  res_container<gl_framebuffer> _framebuffers;

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
  friend class r_window;
};

} // namespace ntf
