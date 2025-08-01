#pragma once

#include <glad/glad.h>

#include "../logger.hpp"

#include <shogle/render/context.hpp>
#include <shogle/render/texture.hpp>
#include <shogle/render/buffer.hpp>
#include <shogle/render/framebuffer.hpp>
#include <shogle/render/pipeline.hpp>

#include <ntfstl/ptr.hpp>
#include <ntfstl/allocator.hpp>
#include <ntfstl/unique_array.hpp>
#include <ntfstl/hashmap.hpp>

#include <deque>
#include <queue>
#include <variant>
#include <atomic>

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

#define RET_ERROR_FMT(_alloc, _fmt, ...) \
  SHOGLE_LOG(error, _fmt, __VA_ARGS__); \
  return ntf::unexpected{render_error{_alloc.fmt_arena_string(_fmt __VA_OPT__(,) __VA_ARGS__)}}

#define RET_ERROR_CATCH(_msg) \
  catch (render_error& err) { \
    SHOGLE_LOG(error, _msg ": {}", err.what()); \
    return {ntf::unexpect, std::move(err)}; \
  } catch (const std::bad_alloc&) { \
    SHOGLE_LOG(error, _msg ": Allocation failed"); \
    return {ntf::unexpect, render_error::alloc_failure}; \
  } catch (...) { \
    SHOGLE_LOG(error, _msg ": Caught (...)"); \
    return {ntf::unexpect, render_error::unknown_error}; \
  }

#define RENDER_ERROR_LOG_CATCH(_msg) \
  catch (const std::exception& ex) { \
    SHOGLE_LOG(error, _msg ": {}", ex.what()); \
  } catch (...) { \
    SHOGLE_LOG(error, _msg ": Caught (...)"); \
  }

namespace shogle {

class ctx_alloc {
public:
  using malloc_fun = void*(*)(void* user_ptr, size_t size, size_t alignment);
  using mfree_fun  = void (*)(void* user_ptr, void* mem, size_t size);

  template<typename T> 
  struct alloc_del_t {
    alloc_del_t(void* user_ptr, mfree_fun mfree) noexcept :
      _uptr{user_ptr}, _free{mfree} {}

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(U* ptr) noexcept(std::is_nothrow_destructible_v<T>)
    {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        static_cast<T*>(ptr)->~T();
      }
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(ntf::uninitialized_t, U* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
    {
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(ntf::uninitialized_t, U* ptr, size_t n)
    noexcept(std::is_nothrow_destructible_v<T>)
    {
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

    adaptor_t(ctx_alloc& alloc) noexcept :
      _alloc{&alloc} {}

    template<typename U>
    adaptor_t(const adaptor_t<U>& other) noexcept :
      _alloc{other._alloc} {}

    adaptor_t(const adaptor_t&) noexcept = default;
    
    pointer allocate(size_t n) {
      return _alloc->allocate_uninited<T>(n);
    }

    void deallocate(pointer ptr, size_t n) noexcept {
      _alloc->deallocate(ptr, n*sizeof(T));
    }

    constexpr bool operator==(const adaptor_t&) { return true; }
    constexpr bool operator!=(const adaptor_t&) { return false; }

    ctx_alloc* _alloc;
  };

  template<typename T>
  struct arena_adaptor_t {
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    template<typename U>
    using rebind = adaptor_t<U>;

    arena_adaptor_t(ctx_alloc& alloc) noexcept :
      _alloc{&alloc} {}

    template<typename U>
    arena_adaptor_t(const arena_adaptor_t<U>& other) noexcept :
      _alloc{other._alloc} {}

    arena_adaptor_t(const arena_adaptor_t&) noexcept = default;
    
    pointer allocate(size_t n) {
      return _alloc->arena_allocate_uninited<T>(n);
    }

    void deallocate(pointer ptr, size_t n) noexcept {
      NTF_UNUSED(ptr);
      NTF_UNUSED(n);
    }

    constexpr bool operator==(const arena_adaptor_t&) { return true; }
    constexpr bool operator!=(const arena_adaptor_t&) { return false; }

    ctx_alloc* _alloc;
  };

  // template<typename T>
  // using adaptor_t = allocator_adaptor<T, rp_alloc>;

  template<typename T>
  using uptr_t = std::unique_ptr<T, alloc_del_t<T>>;

  template<typename T>
  using uarray_t = ntf::unique_array<T, alloc_del_t<T>>;

  template<typename T>
  using vec_t = std::vector<T, adaptor_t<T>>;

  template<typename T>
  using deque_t = std::deque<T, adaptor_t<T>>;

  template<typename T>
  using queue_t = std::queue<T, deque_t<T>>;

  using string_t = std::basic_string<char, std::char_traits<char>, adaptor_t<char>>;

  // template<typename T>
  // struct string_hash {
  //   size_t operator()(const string_t<T>& str) const {
  //     std::hash<cstring_view<T>> h{};
  //     cstring_view<T> view{str};
  //     return h(view);
  //   }
  // };

  template<typename T>
  using string_fhashmap_t = std::unordered_map<
    string_t, T,
    std::hash<string_view>, std::equal_to<string_view>,
    // rp_alloc::alloc_del_t<std::pair<const std::string, r_uniform_>>
    // allocator_delete<
    //   std::pair<const string_t<U>, T>,
      adaptor_t<std::pair<const string_t, T>>
    // >
  >;

public:
  ctx_alloc(const ntf::malloc_funcs& funcs, ntf::linked_arena&& arena) noexcept;

public:
  static uptr_t<ctx_alloc> make_alloc(weak_ptr<const ntf::malloc_funcs> alloc, size_t arena_size);
  
public: 
  void* allocate(size_t size, size_t alignment);
  void deallocate(void* mem, size_t sz);

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

  string_t make_string(size_t reserve = 0u) {
    string_t str{make_adaptor<char>()};
    if (reserve){
      str.reserve(reserve);
    }
    return str;
  }

  template<typename T>
  string_fhashmap_t<T> make_string_map(size_t sz) {
    string_fhashmap_t<T> map{make_adaptor<std::pair<const string_t, T>>()};
    map.reserve(sz);
    return map;
  }

  string_t vfmt_string(fmt::string_view fmt, fmt::format_args args) {
    auto adaptor = make_adaptor<char>();
    fmt::basic_memory_buffer<char, fmt::inline_buffer_size, adaptor_t<char>> buff{adaptor};
    fmt::vformat_to(std::back_inserter(buff), fmt, args);
    return string_t{buff.data(), buff.size(), adaptor};
  }

  template<typename... Args>
  string_t fmt_string(fmt::string_view fmt, Args&&... args) {
    return vfmt_string(fmt, fmt::make_format_args(std::forward<Args>(args)...));
  }

public:
  void arena_clear();
  void* arena_allocate(size_t size, size_t alignment);

  template<typename T, typename... Args>
  T* arena_construct(Args&&... args) {
    return _arena.construct<T>(std::forward<Args>(args)...);
  }

  template<typename T>
  T* arena_allocate_uninited(size_t n = 1u) {
    return static_cast<T*>(arena_allocate(n*sizeof(T), alignof(T)));
  }

public:
  template<typename T>
  span<T> arena_span(size_t count) {
    return span<T>{arena_allocate_uninited<T>(count), count};
  }

  string_view vfmt_arena_string(fmt::string_view fmt, fmt::format_args args) {
    arena_adaptor_t<char> adaptor{*this};
    fmt::basic_memory_buffer<char, 0u, arena_adaptor_t<char>> buff{adaptor};
    fmt::vformat_to(std::back_inserter(buff), fmt, args);
    return {buff.data(), buff.size()};
  }

  template<typename... Args>
  string_view fmt_arena_string(fmt::string_view fmt, Args&&... args) {
    return vfmt_arena_string(fmt, fmt::make_format_args(std::forward<Args>(args)...));
  }

private:
  ntf::linked_arena _arena;
  void* _user_ptr;
  malloc_fun _malloc;
  mfree_fun _free;
};

constexpr bool check_handle(ctx_handle handle) noexcept { return handle != CTX_HANDLE_TOMB; }

using ctx_buff = ctx_handle;
using ctx_tex = ctx_handle;
using ctx_shad = ctx_handle;
using ctx_pip = ctx_handle;
using ctx_unif = ctx_handle;

using ctx_fbo = ctx_handle;
constexpr auto DEFAULT_FBO_HANDLE = CTX_HANDLE_TOMB;

struct ctx_unif_meta {
  ctx_unif handle;
  ctx_alloc::string_t name;
  attribute_type type;
  size_t size;
};
using unif_meta_vec = ctx_alloc::vec_t<ctx_unif_meta>;

template<typename T>
using handle_map = std::unordered_map<ctx_handle, T>;

struct ctx_limits {
  uint32 tex_max_layers;
  uint32 tex_max_extent;
  uint32 tex_max_extent3d;
};

class ctx_render_cmd {
public:
  struct unif_const_t {
    u32 location;
    attribute_data data;
    attribute_type type;
  };

  struct shad_bind_t {
    ctx_buff handle;
    uint32 binding;
    size_t offset;
    size_t size;
  };

  struct tex_bind_t {
    ctx_tex handle;
    u32 sampler;
  };

  static constexpr u32 MAX_LAYOUT_NUMBER = 32u; // hack

public:
  ctx_render_cmd(ntf::function_view<void(context_t)> on_render_) :
    external{ntf::nullopt}, on_render{on_render_} {}
  ctx_render_cmd(ntf::function_view<void(context_t, ctx_handle)> on_render_) :
    external{ntf::nullopt}, on_render{on_render_} {}

public:
  ctx_pip pip;
  std::array<ctx_buff, MAX_LAYOUT_NUMBER> vbo;
  ctx_buff ebo;
  span<shad_bind_t> shader_buffers;
  span<tex_bind_t> textures;
  span<unif_const_t> uniforms;
  render_opts opts;
  uint32 sort_group;
  ntf::optional<external_state> external;
  std::variant<
    ntf::function_view<void(context_t, ctx_handle)>,
    ntf::function_view<void(context_t)>
  > on_render;
};

struct ctx_render_data {
  struct fbo_data_t {
    color4 clear_color;
    uvec4 viewport;
    clear_flag clear_flags;
  };

  ctx_fbo target;
  weak_ptr<const fbo_data_t> data;
  span<const ctx_render_cmd> commands;
};

struct ctx_buff_desc {
  buffer_type type;
  buffer_flag flags;
  size_t size;
  weak_ptr<const buffer_data> data;
};

struct ctx_tex_desc {
  texture_type type;
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  span<const image_data> images;
  bool gen_mipmaps;
};

using ctx_tex_data = texture_data;

struct ctx_tex_opts {
  texture_sampler sampler;
  texture_addressing addresing;
};

struct ctx_shad_desc {
  shader_type type;
  string_view source;
};

using shad_err_str = string_view; 

struct ctx_pip_desc {
  ctx_alloc::uarray_t<attribute_binding> layout;
  weak_ptr<unif_meta_vec> uniforms;
  span<const ctx_shad> stages;
  stages_flag stages_flags;
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests tests;
};

using pip_err_str = string_view; 

struct ctx_fbo_desc {
  struct tex_att_t {
    ctx_tex texture;
    uint32 layer;
    uint32 level;
  };

  extent2d extent;
  fbo_buffer test_buffer;
  span<const tex_att_t> ctx_attachments;
  ctx_alloc::uarray_t<fbo_image> attachments;
};

template<typename T>
class ctx_res_node {
public:
  ctx_res_node(context_t ctx_) noexcept :
    ctx{ctx_}, prev{nullptr}, next{nullptr} {}

public:
  context_t ctx;
  T *prev, *next;
};


using ctx_status = render_error::code_t;

struct icontext {
  virtual ~icontext() = default;
  virtual void get_limits(ctx_limits& limits) = 0;
  virtual ctx_alloc::string_t get_name(ctx_alloc& alloc) = 0;
  virtual void get_dfbo_params(extent2d& ext, fbo_buffer& buff, u32& msaa) = 0;

  virtual ctx_status create_buffer(ctx_buff& buff, const ctx_buff_desc& desc) = 0;
  virtual ctx_status update_buffer(ctx_buff buff, const buffer_data& data) = 0;
  virtual ctx_status map_buffer(ctx_buff buff, void** ptr, size_t size, size_t offset) = 0;
  virtual ctx_status unmap_buffer(ctx_buff buff, void* ptr) noexcept = 0;
  virtual ctx_status destroy_buffer(ctx_buff buff) noexcept = 0;

  virtual ctx_status create_texture(ctx_tex& tex, const ctx_tex_desc& desc) = 0;
  virtual ctx_status update_texture(ctx_tex tex, const ctx_tex_data& data) = 0;
  virtual ctx_status update_texture(ctx_tex tex, const ctx_tex_opts& opts) = 0;
  virtual ctx_status destroy_texture(ctx_tex tex) noexcept = 0;

  virtual ctx_status create_shader(ctx_shad& shad, shad_err_str& err,
                                        const ctx_shad_desc& desc) = 0;
  virtual ctx_status destroy_shader(ctx_shad shad) noexcept = 0;

  virtual ctx_status create_pipeline(ctx_pip& pip, pip_err_str& err,
                                         const ctx_pip_desc& desc) = 0;
  virtual ctx_status destroy_pipeline(ctx_pip pip) noexcept = 0;

  virtual ctx_status create_framebuffer(ctx_fbo& fbo, const ctx_fbo_desc& desc) = 0;
  virtual ctx_status destroy_framebuffer(ctx_fbo fbo) noexcept = 0;

  virtual void submit_render_data(context_t ctx, span<const ctx_render_data> render_data) = 0;

  virtual void device_wait() = 0;
  virtual void swap_buffers() = 0;
};

} // namespace shogle
