#pragma once

#include "./texture.hpp"
#include "./shader.hpp"
#include "./mesh.hpp"
#include "./framebuffer.hpp"
#include "./font.hpp"

#ifdef SHOGLE_ENABLE_IMGUI
#include <imgui_impl_opengl3.h>
#endif

namespace ntf {

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
  template<typename Proc>
  bool init(Proc proc) {
    if (!gladLoadGLLoader((GLADloadproc)proc)) {
      _glad_proc = (GLADloadproc)proc;
      return false;
    }

    return true;
  }

  void destroy();

public:
  mesh make_quad(mesh_buffer vert_buff, mesh_buffer ind_buff);
  mesh make_cube(mesh_buffer vert_buff, mesh_buffer ind_buff);

public:
  void draw(mesh_primitive prim, const mesh& mesh, std::size_t offset = 0, uint count = 0);
  void draw_instanced(mesh_primitive prim, const mesh& mesh, uint primcount,
                      std::size_t offset = 0, uint count = 0);

public:
  void set_viewport(std::size_t w, std::size_t h);
  void set_viewport(ivec2 sz);
  void set_viewport(std::size_t x, std::size_t y, std::size_t w, std::size_t h);
  void set_viewport(ivec2 pos, ivec2 sz);

  void clear_viewport(color4 color, clear flag = clear::none);
  void clear_viewport(color3 color, clear flag = clear::none);

  void set_stencil_test(bool flag);
  void set_depth_test(bool flag);
  void set_blending(bool flag);

  void set_depth_fun(depth_fun fun);

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

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(gl_context);

public:
  friend class glfw_window<gl_context>;
};

} // namespace ntf
