#pragma once

#include "../../core.hpp"

#include <glad/glad.h>

#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
#include <imgui.h>
#include <imgui_impl_opengl3.h> // Should work fine with OGL 4.x
#endif

#if defined(SHOGLE_USE_GLFW) && SHOGLE_USE_GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define SHOGLE_GL_LOAD_PROC \
  reinterpret_cast<GLADloadproc>(glfwGetProcAddress)
#define SHOGLE_GL_MAKE_CTX_CURRENT(win_handle) \
  glfwMakeContextCurrent(win_handle)
#define SHOGLE_GL_SET_SWAP_INTERVAL(win_handle, interval) \
  glfwSwapInterval(interval)
#define SHOGLE_GL_SWAP_BUFFERS(win_handle) \
  glfwSwapBuffers(win_handle)
#define SHOGLE_VK_CREATE_SURFACE(win_handle, vk_instance, vk_surface_ptr, vk_alloc_ptr) \
  glfwCreateWindowSurface(vk_instance, win_handle, vk_alloc_ptr, vk_surface_ptr)

// TODO: Define inits for vulkan & software renderer
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
#include <imgui_impl_glfw.h>
#define SHOGLE_INIT_IMGUI_OPENGL(win, cbks, glsl_ver) \
  ImGui_ImplGlfw_InitForOpenGL(win, cbks); \
  ImGui_ImplOpenGL3_Init(glsl_ver)
#define SHOGLE_INIT_IMGUI_VULKAN(win, cbks)
#define SHOGLE_INIT_IMGUI_OTHER(win, cbks)

#define SHOGLE_DESTROY_IMGUI_OPENGL() \
  ImGui_ImplOpenGL3_Shutdown(); \
  ImGui_ImplGlfw_Shutdown()
#define SHOGLE_DESTROY_IMGUI_VULKAN()
#define SHOGLE_DESTROY_IMGUI_OTHER()

#define SHOGLE_IMGUI_OPENGL_NEW_FRAME() \
  ImGui_ImplGlfw_NewFrame(); \
  ImGui_ImplOpenGL3_NewFrame()
#define SHOGLE_IMGUI_VULKAN_NEW_FRAME()
#define SHOGLE_IMGUI_OTHER_NEW_FRAME()

#define SHOGLE_IMGUI_OPENGL_END_FRAME(draw_data) \
  ImGui_ImplOpenGL3_RenderDrawData(draw_data)
#define SHOGLE_IMGUI_VULKAN_END_FRAME(draw_data)
#define SHOGLE_IMGUI_OTHER_END_FRAME(draw_data)
#endif
#endif

#include "../context.hpp"

#define SHOGLE_DECLARE_RENDER_HANDLE(_name) \
class _name { \
public: \
  constexpr _name() : _handle{r_handle_tombstone} {} \
  constexpr explicit _name(r_handle_value handle) : _handle{handle} {} \
public: \
  constexpr explicit operator r_handle_value() const noexcept { return _handle; } \
  constexpr bool valid() const noexcept { return _handle != r_handle_tombstone; } \
  constexpr explicit operator bool() const noexcept { return valid(); } \
  constexpr bool operator==(const _name& rhs) const noexcept { \
    return _handle == static_cast<r_handle_value>(rhs); \
  } \
private: \
  r_handle_value _handle; \
}; \

namespace ntf {

using r_handle_value = uint32;
constexpr r_handle_value r_handle_tombstone = std::numeric_limits<r_handle_value>::max();

template<typename T>
struct r_handle_hash {
  r_handle_hash() noexcept = default;
  size_t operator()(const T& handle) const noexcept {
    return std::hash<r_handle_value>{}(static_cast<r_handle_value>(handle));
  }
};

SHOGLE_DECLARE_RENDER_HANDLE(r_platform_buffer);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_texture);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_shader);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_pipeline);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_uniform);
SHOGLE_DECLARE_RENDER_HANDLE(r_platform_fbo);

constexpr r_platform_fbo DEFAULT_FBO_HANDLE{r_handle_tombstone};

template<typename K, typename T>
using handle_map = std::unordered_map<K, T, r_handle_hash<K>>;

enum class rp_test_buffer_flag : uint8 {
  none = 0,
  depth,
  stencil,
};

struct vertex_layout {
  uint32 binding;
  size_t stride;
  std::vector<r_attrib_descriptor> descriptors;
};

using uniform_map = std::unordered_map<std::string, r_uniform>;

struct uniform_descriptor {
  r_attrib_type type;
  r_platform_uniform location;
  const void* data;
  size_t size;
};

struct texture_binding {
  r_texture handle;
  uint32 index;
};

struct draw_command {
  r_pipeline pipeline;
  std::vector<weak_ref<texture_binding>> textures;
  std::vector<weak_ref<uniform_descriptor>> uniforms;
  std::vector<weak_ref<r_buffer_binding>> buffers;
  uint32 count;
  uint32 offset;
  uint32 instances;
  std::function<void()> on_render;
};

struct draw_list {
  color4 color;
  uvec4 viewport;
  r_clear_flag clear;
  std::vector<weak_ref<draw_command>> cmds;
};

using command_map = handle_map<r_platform_fbo, draw_list>;

struct rp_platform_meta {
  r_api api;
  std::string name_str;
  std::string vendor_str;
  std::string version_str;
  uint32 tex_max_layers;
  uint32 tex_max_extent;
  uint32 tex_max_extent3d;
};

struct rp_buff_desc {
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;
  weak_cref<r_buffer_data> initial_data;
};

struct rp_buff_data {
  const void* data;
  size_t len;
  size_t offset;
};

struct rp_buff_mapping {
  size_t len;
  size_t offset;
};

struct rp_tex_desc {
  r_texture_type type;
  r_texture_format format;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  cspan<r_image_data> initial_data;
  bool gen_mipmaps;
  r_texture_sampler sampler;
  r_texture_address addressing;
};

struct rp_tex_image_data {
  const void* texels;
  r_texture_format format;
  r_image_alignment alignment;
  extent3d extent;
  extent3d offset;
  uint32 layer;
  uint32 level;
};

struct rp_tex_opts {
  r_texture_sampler sampler;
  r_texture_address addressing;
  bool regen_mips;
};

struct rp_shad_desc {
  r_shader_type type;
  cspan<std::string_view> source;
};

struct rp_pip_desc {
  std::unique_ptr<vertex_layout> layout;
  weak_ref<uniform_map> uniforms;

  cspan<r_platform_shader> stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;

  weak_cref<r_stencil_test_opts> stencil_test;
  weak_cref<r_depth_test_opts> depth_test;
  weak_cref<r_scissor_test_opts> scissor_test;
  weak_cref<r_face_cull_opts> face_culling;
  weak_cref<r_blend_opts> blending;
};

struct rp_pip_opts {
  weak_cref<r_stencil_test_opts> stencil_test;
  weak_cref<r_depth_test_opts> depth_test;
  weak_cref<r_scissor_test_opts> scissor_test;
  weak_cref<r_face_cull_opts> face_culling;
  weak_cref<r_blend_opts> blending;
};

struct rp_fbo_att {
  r_platform_texture texture;
  uint32 layer;
  uint32 level;
};

struct rp_fbo_desc {
  extent2d extent;
  uvec4 viewport;
  r_test_buffer test_buffer;
  std::variant<cspan<rp_fbo_att>, r_texture_format> attachments;
};


struct r_platform_context {
  virtual ~r_platform_context() = default;
  virtual void get_meta(rp_platform_meta& meta) = 0;

  virtual r_platform_buffer create_buffer(const rp_buff_desc& desc) = 0;
  virtual void update_buffer(r_platform_buffer buf, const rp_buff_data& data) = 0;
  virtual void* map_buffer(r_platform_buffer buf, const rp_buff_mapping& mapping) = 0;
  virtual void unmap_buffer(r_platform_buffer buf, void* ptr) noexcept = 0;
  virtual void destroy_buffer(r_platform_buffer buf) noexcept = 0;

  virtual r_platform_texture create_texture(const rp_tex_desc& desc) = 0;
  virtual void update_texture_image(r_platform_texture tex, const rp_tex_image_data& image) = 0;
  virtual void update_texture_options(r_platform_texture tex, const rp_tex_opts& opts) = 0;
  virtual void destroy_texture(r_platform_texture tex) noexcept = 0;

  virtual r_platform_shader create_shader(const rp_shad_desc& desc) = 0;
  virtual void destroy_shader(r_platform_shader shader) noexcept = 0;

  virtual r_platform_pipeline create_pipeline(const rp_pip_desc& desc) = 0;
  virtual void update_pipeline_options(r_pipeline pip, const rp_pip_opts& opts) = 0;
  virtual void destroy_pipeline(r_platform_pipeline pipeline) noexcept = 0;

  virtual r_platform_fbo create_framebuffer(const rp_fbo_desc& desc) = 0;
  virtual void destroy_framebuffer(r_platform_fbo fb) noexcept = 0;

  virtual void submit(const command_map& cmds) = 0;

  virtual void device_wait() noexcept {}
  virtual void swap_buffers() = 0;
};

r_platform_buffer r_buffer_get_handle(r_buffer buff);
r_platform_texture r_texture_get_handle(r_texture tex);
r_platform_fbo r_framebuffer_get_handle(r_framebuffer fbo);
r_platform_shader r_shader_get_handle(r_shader shader);
r_platform_pipeline r_pipeline_get_handle(r_pipeline pip);

} // namespace ntf

#undef SHOGLE_DECLARE_RENDER_HANDLE
