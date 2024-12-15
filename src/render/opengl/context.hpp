#pragma once

#include "./texture.hpp"
#include "./shader.hpp"
#include "./mesh.hpp"
#include "./framebuffer.hpp"
#include "./font.hpp"

#include "buffer.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui_impl_opengl3.h>
#endif

#include <optional>
#include <vector>

namespace ntf {

using gl_handle = uint32_t;
using gl_id = GLuint;

class gl_context {
private:

  struct gl_buffer {
    gl_id id;
  };

  struct gl_mesh {
    gl_id vao;
  };

  struct gl_shader {
    gl_id id;
  };
  
  struct gl_pipeline {
    gl_id id;
  };

  struct gl_texture {
    gl_id id;
  };


public:
  gl_context() = default;

public:
  bool init(GLADloadproc proc);

  void destroy();

  void update_viewport(std::size_t w, std::size_t h);
  void update_viewport(std::size_t x, std::size_t y, std::size_t w, std::size_t h);

  void stencil_test(bool flag);
  void depth_test(bool flag);
  void blending(bool flag);

  void clear(color4 color, clear flag);

public:
  std::vector<gl_buffer> _buffers;
  std::vector<gl_mesh> _meshes;
  std::vector<gl_shader> _shaders;
  std::vector<gl_pipeline> _pipelines;
  std::vector<gl_texture> _texture;


};

template<typename RenderContext>
class glfw_window;

class gl_context {
public:
  template<std::size_t faces>
  using texture = gl_texture<faces>;

  using texture2d = gl_texture<1u>;
  using cubemap = gl_texture<6u>;

  using shader = gl_shader;
  using uniform = gl_shader_uniform;
  using program = gl_shader_program;

  using font = gl_font;

  using mesh = gl_mesh;

  using framebuffer = gl_framebuffer;

  using vertex_buffer = ::ntf::vertex_buffer<gl_context>;
  using index_buffer  = ::ntf::index_buffer<gl_context>;

#ifdef SHOGLE_ENABLE_IMGUI
  struct imgui_impl {
    static bool init(const char* glsl_ver) {
      ImGui_ImplOpenGL3_Init(glsl_ver);
      return true;
    }
    static void destroy() {
      ImGui_ImplOpenGL3_Shutdown();
    }
    static void start_frame() {
      ImGui_ImplOpenGL3_NewFrame();
    }
    static void end_frame() {}

    static constexpr renderer_backend backend_enum = renderer_backend::opengl;
  };
#else
  using imgui_impl = void;
#endif

private:
  gl_context() = default;

  template<typename Proc>
  bool init(Proc proc) {
    NTF_ASSERT(!valid());
    if (!gladLoadGLLoader((GLADloadproc)proc)) {
      _glad_proc = (GLADloadproc)proc;
      return false;
    }

    return true;
  }

  void destroy();

public:
  mesh make_quad(mesh_buffer vert_buff, mesh_buffer ind_buff) const;
  mesh make_cube(mesh_buffer vert_buff, mesh_buffer ind_buff) const;

public:
  void draw(mesh_primitive prim, const mesh& mesh, std::size_t offset = 0, uint count = 0) const;
  void draw_instanced(mesh_primitive prim, const mesh& mesh, uint primcount,
                      std::size_t offset = 0, uint count = 0) const;

  template<std::size_t faces>
  void bind_sampler(const gl_texture<faces>& tex, uint sampler) const {
    if (!tex.valid()) {
      SHOGLE_LOG(warning, "[ntf::gl_context::draw_text] Attempted to bind empty texture");
      return;
    }

    glActiveTexture(GL_TEXTURE0+sampler);
    glBindTexture(gl_texture<faces>::gltype, tex.id());
  }

  void draw_text(const font& font, vec2 pos, float scale, std::string_view text) const;
  void draw_text(const font& font, std::string_view text) const;

  template<typename... Args>
  void draw_text(const font& font, vec2 pos, float scale, fmt::format_string<Args...> fmt,
                 Args&&... args) const {
    std::string str = fmt::format(fmt, std::forward<Args>(args)...);
    draw_text(font, pos, scale, str);
  }

  template<typename... Args>
  void draw_text(const font& font, fmt::format_string<Args...> fmt, Args&&... args) const {
    draw_text(font, vec2{0.f}, 0.f, fmt, std::forward<Args>(args)...);
  }

public:
  void set_viewport(std::size_t w, std::size_t h) const;
  void set_viewport(ivec2 sz) const;
  void set_viewport(std::size_t x, std::size_t y, std::size_t w, std::size_t h) const;
  void set_viewport(ivec2 pos, ivec2 sz) const;

  void clear_viewport(color4 color, clear flag = clear::none) const;
  void clear_viewport(color3 color, clear flag = clear::none) const;

  void set_stencil_test(bool flag) const;
  void set_depth_test(bool flag) const;
  void set_blending(bool flag) const;

  void set_depth_fun(depth_fun fun) const;


public:
  std::optional<vertex_buffer> make_vertex_buffer(void* data, std::size_t size,
                                                  const std::vector<shader_data_type>& attribs);

  std::optional<index_buffer> make_index_buffer(void* data, std::size_t size);


public:
  bool valid() const { return _glad_proc != nullptr; }
  GLADloadproc proc() const { return _glad_proc; }
  const char* name() const { return (const char*)glGetString(GL_RENDERER); }
  const char* vendor() const { return (const char*)glGetString(GL_VENDOR); }
  const char* version() const{ return (const char*)glGetString(GL_VERSION); }

public:
  static gl_context& current_context();

private:
  static void set_current_context(gl_context& ctx);

private:
  GLADloadproc _glad_proc{nullptr};

  std::vector<gl_buffer> _buffers;
  std::vector<gl_vertex_array> _vaos;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(gl_context);

public:
  friend class glfw_window<gl_context>;
};

} // namespace ntf
