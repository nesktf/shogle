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
  glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(win_handle))
#define SHOGLE_GL_SET_SWAP_INTERVAL(win_handle, interval) \
  glfwSwapInterval(interval)
#define SHOGLE_GL_SWAP_BUFFERS(win_handle) \
  glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(win_handle))
#define SHOGLE_VK_CREATE_SURFACE(win_handle, vk_instance, vk_surface_ptr, vk_alloc_ptr) \
  glfwCreateWindowSurface(vk_instance, reinterpret_cast<GLFWwindow*>(win_handle), \
                          vk_alloc_ptr, vk_surface_ptr)

// TODO: Define inits for vulkan & software renderer
#if defined(SHOGLE_ENABLE_IMGUI) && SHOGLE_ENABLE_IMGUI
#include <imgui_impl_glfw.h>
#define SHOGLE_INIT_IMGUI_OPENGL(win, cbks) \
  ImGui_ImplGlfw_InitForOpenGL(reinterpret_cast<GLFWwindow*>(win), cbks); \
  ImGui_ImplOpenGL3_Init("version #150")
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

#include "../../stl/arena.hpp"

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

class rp_alloc {
private:
  static r_allocator base_alloc;

public:
  template<typename T>
  using adaptor_t = allocator_adaptor<T, rp_alloc>;

public:
  rp_alloc() noexcept :
    _arena{nullptr, 0u},
    _user_ptr{base_alloc.user_ptr},
    _malloc{base_alloc.mem_alloc}, _free{base_alloc.mem_free} {}

  rp_alloc(const r_allocator& user_alloc) noexcept :
    _arena{nullptr, 0u},
    _user_ptr{user_alloc.user_ptr},
    _malloc{user_alloc.mem_alloc}, _free{user_alloc.mem_free} {}
  
public: 
  void* allocate(size_t size, size_t alignment) {
    return std::invoke(_malloc, _user_ptr, size, alignment);
  }

  void deallocate(void* mem, size_t) {
    std::invoke(_free, _user_ptr, mem);
  }

  template<typename T>
  T* allocate_uninited(size_t n = 1u) {
    return static_cast<T*>(allocate(n*sizeof(T), alignof(T)));
  }

  template<typename T, typename... Args>
  T* construct(Args&&... args) {
    T* obj = static_cast<T*>(allocate(sizeof(T), alignof(T)));
    if (!obj) {
      return nullptr;
    }
    std::construct_at(obj, std::forward<Args>(args)...);
    return obj;
  }

  template<typename T>
  void destroy(T* obj) {
    obj->~T();
    deallocate(obj, 1u);
  }

public:
  bool arena_init(size_t block_sz) {
void* mem = allocate(block_sz, 0u);
    if (!mem) {
      return false;
    }
    _arena = {mem, block_sz};
    return true;
  }

  void arena_destroy() {
    deallocate(_arena.data(), _arena.capacity());
  }

  bool resize_arena(size_t block_sz) {
    void* data = _arena.data();
    size_t cap = _arena.capacity();
    if (arena_init(block_sz)) {
      deallocate(data, cap);
      return true;
    }
    return false;
  }

  void arena_clear() {
    _arena.clear();
  }

  void* arena_allocate(size_t size, size_t alignment) {
    return _arena.allocate(size, alignment);
  }

  template<typename T, typename... Args>
  T* arena_construct(Args&&... args) {
    return _arena.construct<T>(std::forward<Args>(args)...);
  }

  template<typename T>
  T* arena_allocate_uninited(size_t n = 1u) {
    return _arena.allocate<T>(n);
  }

private:
  arena_block_manager _arena;
  void* _user_ptr;
  void* (*_malloc)(void* user_ptr, size_t size, size_t alignment);
  void  (*_free)(void* user_ptr, void* mem);
};

struct rp_uniform_query {
  r_platform_uniform location;
  std::string name;
  r_attrib_type type;
  size_t size;
};
using rp_uniform_query_vec = std::vector<rp_uniform_query, rp_alloc::adaptor_t<rp_uniform_query>>;

struct rp_uniform_binding {
  r_platform_uniform location;
  r_attrib_type type;
  const void* data;
  size_t size;
};

struct rp_texture_binding {
  r_platform_texture tex;
  uint32 index;
};

struct rp_buffer_binding {
  r_platform_buffer buffer;
  r_buffer_type type;
  optional<uint32> location;
};

struct rp_draw_cmd {
  r_context ctx;
  r_platform_pipeline pipeline;
  span<rp_buffer_binding> buffers;
  span<rp_texture_binding> textures;
  span<rp_uniform_binding> uniforms;
  uint32 count;
  uint32 offset;
  uint32 instances;
  uint32 sort_group;
  function_view<void(r_context)> on_render;
};

struct rp_fbo_frame_data {
  color4 clear_color;
  uvec4 viewport;
  r_clear_flag clear_flags;
};

struct rp_draw_data {
  r_platform_fbo target;
  weak_cref<rp_fbo_frame_data> fdata;
  cspan<rp_draw_cmd> cmds;
};

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
  vertex_layout* layout;
  weak_ref<rp_uniform_query_vec> uniforms;

  cspan<r_platform_shader> stages;
  r_stages_flag stages_flags;
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

  virtual void submit(cspan<rp_draw_data> draw_data) = 0;

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
