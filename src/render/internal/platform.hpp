#pragma once

#include "../../core.hpp"

#include <glad/glad.h>

#if defined(SHOGLE_ENABLE_GLFW) && SHOGLE_ENABLE_GLFW
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
#endif

#include "../context.hpp"

#include <ntfstl/allocator.hpp>
#include <ntfstl/unique_array.hpp>

#include <deque>
#include <queue>

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
}

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

class rp_alloc {
public:
  using malloc_fun = void*(*)(void* user_ptr, size_t size, size_t alignment);
  using mfree_fun  = void (*)(void* user_ptr, void* mem, size_t size);

  template<typename T> 
  struct alloc_del_t {
    alloc_del_t(void* user_ptr, mfree_fun mfree) noexcept :
      _uptr{user_ptr}, _free{mfree} {}

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(U* ptr) noexcept(std::is_nothrow_destructible_v<T>) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        static_cast<T*>(ptr)->~T();
      }
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(uninitialized_t, U* ptr) noexcept(std::is_nothrow_destructible_v<T>) {
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(uninitialized_t, U* ptr, size_t n) noexcept(std::is_nothrow_destructible_v<T>) {
      std::invoke(_free, _uptr, ptr, n*sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(U* ptr, size_t n) noexcept(std::is_nothrow_destructible_v<T>) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (U* it = ptr; it < ptr+n; ++it) {
          static_cast<T*>(it)->~T();
        }
      }
      std::invoke(_free, _uptr, ptr, n*sizeof(T));
    }

  private:
    void* _uptr;
    mfree_fun _free;
  };

  template<typename T>
  struct adaptor_t {
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    using rebind = adaptor_t<U>;

    adaptor_t(rp_alloc& alloc) noexcept :
      _alloc{&alloc} {}

    template<typename U>
    adaptor_t(const adaptor_t<U>& other) noexcept :
      _alloc{other._alloc} {}

    adaptor_t(const adaptor_t&) noexcept = default;
    
    pointer allocate(size_t n) {
      T* ptr = _alloc->allocate_uninited<T>(n);
      if (!ptr) {
        throw std::bad_alloc{};
      }
      return ptr;
    }

    void deallocate(pointer ptr, size_t n) noexcept {
      _alloc->deallocate(ptr, n*sizeof(T));
    }

    rp_alloc* _alloc;
  };

  // template<typename T>
  // using adaptor_t = allocator_adaptor<T, rp_alloc>;

  template<typename T>
  using uptr_t = std::unique_ptr<T, alloc_del_t<T>>;

  template<typename T>
  using uarray_t = unique_array<T, alloc_del_t<T>>;

  template<typename T>
  using vec_t = std::vector<T, adaptor_t<T>>;

  template<typename T>
  using deque_t = std::deque<T, adaptor_t<T>>;

  template<typename T>
  using queue_t = std::queue<T, deque_t<T>>;

public:
  rp_alloc(const r_allocator& user_alloc, linked_arena&& arena) noexcept :
    _arena{std::move(arena)},
    _user_ptr{user_alloc.user_ptr},
    _malloc{user_alloc.mem_alloc}, _free{user_alloc.mem_free} {}

public:
  static uptr_t<rp_alloc> make_alloc(weak_cptr<r_allocator> alloc, size_t arena_size);
  
public: 
  void* allocate(size_t size, size_t alignment) {
    return std::invoke(_malloc, _user_ptr, size, alignment);
  }

  void deallocate(void* mem, size_t sz) {
    std::invoke(_free, _user_ptr, mem, sz);
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
    deallocate(obj, sizeof(T));
  }

public:
  template<typename T>
  adaptor_t<T> make_adaptor() noexcept { return adaptor_t<T>{*this}; }

  template<typename T>
  alloc_del_t<T> make_deleter() noexcept { return alloc_del_t<T>{_user_ptr, _free}; }

  template<typename T>
  uptr_t<T> wrap_unique(T* ptr) noexcept {
    return uptr_t<T>{ptr, alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T, typename... Args>
  uptr_t<T> make_unique(Args&&... args) {
    return uptr_t<T>{construct<T>(std::forward<Args>(args)...), alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T>
  uarray_t<T> wrap_array(T* ptr, size_t count) {
    return uarray_t<T>{ptr, count, alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T>
  uarray_t<T> make_uninited_array(size_t count) {
    T* ptr = allocate_uninited<T>(count);
    return uarray_t<T>{ptr ? count : 0u, ptr, alloc_del_t<T>{_user_ptr, _free}};
  }

  template<typename T>
  vec_t<T> make_vector(size_t reserve = 0u) {
    vec_t<T> vec{make_adaptor<T>()};
    if (reserve) {
      vec.reserve(reserve);
    }
    return vec;
  }

public:
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
    return _arena.allocate_uninited<T>(n);
  }

public:
  template<typename T>
  span<T> arena_span(size_t count) {
    auto* ptr = arena_allocate_uninited<T>(count);
    return span<T>{ptr, ptr ? count : 0u};
  }

private:
  linked_arena _arena;
  void* _user_ptr;
  malloc_fun _malloc;
  mfree_fun _free;
};

struct rp_uniform_query {
  r_platform_uniform location;
  std::string name;
  r_attrib_type type;
  size_t size;
};
using rp_uniform_query_vec = rp_alloc::vec_t<rp_uniform_query>;

template<typename K, typename T>
using handle_map = std::unordered_map<K, T, r_handle_hash<K>>;

struct rp_draw_cmd {
public:
  struct uniform_const {
    r_platform_uniform location;
    r_attrib_type type;
    const void* data;
    size_t size;
  };

  struct shader_buffer {
    r_platform_buffer handle;
    uint32 binding;
    size_t offset;
    size_t size;
  };

public:
  rp_draw_cmd(function_view<void(r_context)> on_render_) :
    index_buffer{nullopt}, on_render{on_render_}, external{nullopt} {}

  rp_draw_cmd(function_view<void(r_context, r_platform_handle)> on_render_) :
    index_buffer{nullopt}, on_render{on_render_}, external{nullopt} {}

public:
  r_platform_pipeline pipeline;
  span<r_platform_texture> textures;

  r_platform_buffer vertex_buffer;
  optional<r_platform_buffer> index_buffer;
  span<shader_buffer> shader_buffers;
  span<uniform_const> uniforms;

  r_draw_opts draw_opts;
  uint32 sort_group;

  std::variant<
    function_view<void(r_context, r_platform_handle)>,
    function_view<void(r_context)>
  > on_render;
  optional<r_external_state> external;
};

struct rp_fbo_frame_data {
  color4 clear_color;
  uvec4 viewport;
  r_clear_flag clear_flags;
};

struct rp_draw_data {
  r_platform_fbo target;
  weak_cptr<rp_fbo_frame_data> fdata;
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
  weak_cptr<r_buffer_data> initial_data;
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
};

struct rp_shad_desc {
  r_shader_type type;
  std::string_view source;
};

struct rp_pip_desc {
  rp_alloc::uarray_t<r_attrib_binding> layout;
  weak_ptr<rp_uniform_query_vec> uniforms;

  cspan<r_platform_shader> stages;
  r_stages_flag stages_flags;
  r_primitive primitive;
  r_polygon_mode poly_mode;
  optional<float> poly_width;

  weak_cptr<r_stencil_test_opts> stencil_test;
  weak_cptr<r_depth_test_opts> depth_test;
  weak_cptr<r_scissor_test_opts> scissor_test;
  weak_cptr<r_face_cull_opts> face_culling;
  weak_cptr<r_blend_opts> blending;
};

struct rp_pip_opts {
  weak_cptr<r_stencil_test_opts> stencil_test;
  weak_cptr<r_depth_test_opts> depth_test;
  weak_cptr<r_scissor_test_opts> scissor_test;
  weak_cptr<r_face_cull_opts> face_culling;
  weak_cptr<r_blend_opts> blending;
};

struct rp_fbo_att {
  r_platform_texture texture;
  uint32 layer;
  uint32 level;
};

struct rp_fbo_desc {
  extent2d extent;
  r_test_buffer test_buffer;
  cspan<rp_fbo_att> attachments;
  optional<r_texture_format> color_buffer;
};

// enum class rp_res_type {
//   buffer,
//   texture,
//   framebuffer,
//   shader,
//   pipeline,
// };

struct rp_context {
  virtual ~rp_context() = default;
  virtual void get_meta(rp_platform_meta& meta) = 0;

  virtual r_platform_buffer create_buffer(const rp_buff_desc& desc) = 0;
  virtual void update_buffer(r_platform_buffer buf, const rp_buff_data& data) = 0;
  virtual void* map_buffer(r_platform_buffer buf, const rp_buff_mapping& mapping) = 0;
  virtual void unmap_buffer(r_platform_buffer buf, void* ptr) noexcept = 0;
  virtual void destroy_buffer(r_platform_buffer buf) noexcept = 0;

  virtual r_platform_texture create_texture(const rp_tex_desc& desc) = 0;
  virtual void upload_texture_images(r_platform_texture tex,
                                     cspan<rp_tex_image_data> images, bool regen_mips) = 0;
  virtual void update_texture_options(r_platform_texture tex, const rp_tex_opts& opts) = 0;
  virtual void destroy_texture(r_platform_texture tex) noexcept = 0;

  virtual r_platform_shader create_shader(const rp_shad_desc& desc) = 0;
  virtual void destroy_shader(r_platform_shader shader) noexcept = 0;

  virtual r_platform_pipeline create_pipeline(const rp_pip_desc& desc) = 0;
  virtual void update_pipeline_options(r_platform_pipeline pip, const rp_pip_opts& opts) = 0;
  virtual void destroy_pipeline(r_platform_pipeline pipeline) noexcept = 0;

  virtual r_platform_fbo create_framebuffer(const rp_fbo_desc& desc) = 0;
  virtual void destroy_framebuffer(r_platform_fbo fb) noexcept = 0;

  virtual void submit(r_context ctx, cspan<rp_draw_data> draw_data) = 0;
  // virtual void immediate_bind(r_handle_value handle, rp_res_type type, int32 binding) = 0;

  virtual void device_wait() noexcept {}
  virtual void swap_buffers() = 0;
};

r_platform_buffer r_buffer_get_handle(r_buffer buff);
r_platform_texture r_texture_get_handle(r_texture tex);
r_platform_fbo r_framebuffer_get_handle(r_framebuffer fbo);
r_platform_shader r_shader_get_handle(r_shader shader);
r_platform_pipeline r_pipeline_get_handle(r_pipeline pip);

// void r_assert_binded_fbo(r_framebuffer fbo);

} // namespace ntf

#undef SHOGLE_DECLARE_RENDER_HANDLE
