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

#define RENDER_ERROR_LOG(_fmt, ...) \
  SHOGLE_LOG(error, "{} ({})" _fmt, NTF_FUNC, NTF_LINE __VA_OPT__(,) __VA_ARGS__)

#define RENDER_WARN_LOG(_fmt, ...) \
  SHOGLE_LOG(warning, "{} ({})" _fmt, NTF_FUNC, NTF_LINE __VA_OPT__(,) __VA_ARGS__)

#define RET_ERROR(_fmt, ...) \
  RENDER_ERROR_LOG(_fmt __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{r_error::format({_fmt} __VA_OPT__(,) __VA_ARGS__)}

#define RET_ERROR_IF(_cond, _fmt, ...) \
  if (_cond) { \
    RET_ERROR(_fmt, __VA_ARGS__); \
  }

#define RET_ERROR_CATCH(_msg) \
  catch (r_error& err) { \
    RENDER_ERROR_LOG(_msg ": {}", err.what()); \
    return unexpected{std::move(err)}; \
  } catch (const std::exception& ex) { \
    RENDER_ERROR_LOG(_msg ": {}", ex.what()); \
    return unexpected{r_error::format({"{}"}, ex.what())}; \
  } catch (...) { \
    RENDER_ERROR_LOG(_msg ": Caught (...)"); \
    return unexpected{r_error{"Unknown error"}}; \
  }

#define RENDER_ERROR_LOG_CATCH(_msg) \
  catch (const std::exception& ex) { \
    RENDER_ERROR_LOG(_msg ": {}", ex.what()); \
  } catch (...) { \
    RENDER_ERROR_LOG(_msg ": Caught (...)"); \
  }

#include "../context.hpp"
#include "../buffer.hpp"
#include "../framebuffer.hpp"
#include "../texture.hpp"
#include "../pipeline.hpp"

#include <ntfstl/allocator.hpp>
#include <ntfstl/unique_array.hpp>
#include <ntfstl/hashmap.hpp>

#include <variant>
#include <deque>
#include <queue>
#include <atomic>

namespace ntf::render {

constexpr bool check_handle(ctx_handle handle) noexcept { return handle != CTX_HANDLE_TOMB; }

using ctx_buff = ctx_handle;
using ctx_tex = ctx_handle;
using ctx_shad = ctx_handle;
using ctx_pip = ctx_handle;
using ctx_unif = ctx_handle;

using ctx_fbo = ctx_handle;
constexpr auto DEFAULT_FBO_HANDLE = CTX_HANDLE_TOMB;

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
    void operator()(uninitialized_t, U* ptr)
    noexcept(std::is_nothrow_destructible_v<T>)
    {
      std::invoke(_free, _uptr, ptr, sizeof(T));
    }

    template<typename U = T>
    requires(std::is_convertible_v<U*, T*>)
    void operator()(uninitialized_t, U* ptr, size_t n)
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
      T* ptr = _alloc->allocate_uninited<T>(n);
      if (!ptr) {
        throw std::bad_alloc{};
      }
      return ptr;
    }

    void deallocate(pointer ptr, size_t n) noexcept {
      _alloc->deallocate(ptr, n*sizeof(T));
    }

    constexpr bool operator==(const adaptor_t&) { return true; }
    constexpr bool operator!=(const adaptor_t&) { return false; }

  private:
    ctx_alloc* _alloc;
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

  template<typename T>
  using string_t = std::basic_string<T, std::char_traits<T>, adaptor_t<T>>;

  template<typename T>
  using string_view_t = std::basic_string_view<T, std::char_traits<T>>;

  template<typename T>
  struct string_hash {
    size_t operator()(const string_t<T>& str) const {
      std::hash<string_view_t<T>> h{};
      string_view_t<T> view{str};
      return h(view);
    }
  };

  template<typename T, typename U = char>
  using string_fhashmap_t = fixed_hashmap<
    string_t<U>, T,
    string_hash<U>, std::equal_to<string_t<U>>,
    // rp_alloc::alloc_del_t<std::pair<const std::string, r_uniform_>>
    allocator_delete<
      std::pair<const string_t<U>, T>,
      adaptor_t<std::pair<const string_t<U>, T>>
    >
  >;

public:
  ctx_alloc(const malloc_funcs& funcs, linked_arena&& arena) noexcept :
    _arena{std::move(arena)},
    _user_ptr{funcs.user_ptr},
    _malloc{funcs.mem_alloc}, _free{funcs.mem_free} {}

public:
  static uptr_t<ctx_alloc> make_alloc(weak_cptr<malloc_funcs> alloc, size_t arena_size);
  
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

  template<typename T = char>
  string_t<T> make_string(size_t reserve = 0u) {
    string_t<T> str{make_adaptor<T>()};
    if (reserve){
      str.reserve(reserve);
    }
    return str;
  }

  template<typename T = char>
  string_t<T> fmt_string(fmt::string_view fmt, fmt::format_args args) {
    auto adaptor = make_adaptor<T>();
    fmt::basic_memory_buffer<T, fmt::inline_buffer_size, adaptor_t<T>> buff{adaptor};
    fmt::vformat_to(std::back_inserter(buff), fmt, args);
    return string_t<T>{buff.data(), buff.size(), adaptor};
  }

  template<typename T = char, typename... Args>
  string_t<T> fmt_string_args(fmt::string_view fmt, Args&&... args) {
    return fmt_string(fmt, fmt::make_format_args(std::forward<Args>(args)...));
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

struct ctx_unif_meta {
  ctx_unif handle;
  ctx_alloc::string_t<char> name;
  attribute_type type;
  size_t size;
};
using unif_meta_vec = ctx_alloc::vec_t<ctx_unif_meta>;

template<typename T>
using handle_map = std::unordered_map<ctx_handle, T>;

struct ctx_meta {
  struct limits_t {
    uint32 tex_max_layers;
    uint32 tex_max_extent;
    uint32 tex_max_extent3d;
  };

  context_api api;
  ctx_alloc::string_t<char> name_str;
  limits_t limits;
};

class ctx_render_cmd {
public:
  struct unif_const_t {
    ctx_unif handle;
    const void* data;
    attribute_type type;
    size_t size;
  };

  struct shad_bind_t {
    ctx_buff handle;
    uint32 binding;
    size_t offset;
    size_t size;
  };

public:
  ctx_render_cmd(function_view<void(context_t)> on_render_) :
    external{nullopt}, on_render{on_render_} {}
  ctx_render_cmd(function_view<void(context_t, ctx_handle)> on_render_) :
    external{nullopt}, on_render{on_render_} {}

public:
  ctx_pip pip;
  ctx_buff vbo;
  ctx_buff ebo;
  span<shad_bind_t> shader_buffers;
  span<ctx_tex> textures;
  span<unif_const_t> uniforms;
  render_opts opts;
  uint32 sort_group;
  optional<external_state> external;
  std::variant<
    function_view<void(context_t, ctx_handle)>,
    function_view<void(context_t)>
  > on_render;
};

struct ctx_render_data {
  struct fbo_data_t {
    color4 clear_color;
    uvec4 viewport;
    clear_flag clear_flags;
  };

  ctx_fbo target;
  weak_cptr<fbo_data_t> data;
  cspan<ctx_render_cmd> commands;
};

struct ctx_buff_desc {
  buffer_type type;
  buffer_flag flags;
  size_t size;
  weak_cptr<buffer_data> data;
};

enum ctx_buff_status {
  CTX_BUFF_STATUS_OK = 0,
  CTX_BUFF_STATUS_INVALID_HANDLE,
  CTX_BUFF_STATUS_ALLOC_FAILED,
};

struct ctx_tex_desc {
  texture_type type;
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 layers;
  uint32 levels;
  cspan<image_data> images;
  bool gen_mipmaps;
};

using ctx_tex_data = texture_data;

struct ctx_tex_opts {
  texture_sampler sampler;
  texture_addressing addresing;
};

enum ctx_tex_status {
  CTX_TEX_STATUS_OK = 0,
  CTX_TEX_STATUS_INVALID_HANDLE,
  CTX_TEX_STATUS_INVALID_ADDRESING,
  CTX_TEX_STATUS_INVALID_SAMPLER,
  CTX_TEX_STATUS_INVALID_LEVELS,
};

struct ctx_shad_desc {
  shader_type type;
  std::string_view source;
};

using shad_err_str = ctx_alloc::string_view_t<char>;

enum ctx_shad_status {
  CTX_SHAD_STATUS_OK = 0,
  CTX_SHAD_STATUS_INVALID_HANDLE,
  CTX_SHAD_STATUS_COMPILATION_FAILED,
};

struct ctx_pip_desc {
  ctx_alloc::uarray_t<attribute_binding> layout;
  weak_ptr<unif_meta_vec> uniforms;
  cspan<ctx_shad> stages;
  stages_flag stages_flags;
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests tests;
};

using pip_err_str = ctx_alloc::string_view_t<char>;

enum ctx_pip_status {
  CTX_PIP_STATUS_OK = 0,
  CTX_PIP_STATUS_INVALID_HANDLE,
  CTX_PIP_STATUS_LINKING_FAILED,
};

struct ctx_fbo_desc {
  struct tex_att_t {
    ctx_tex texture;
    uint32 layer;
    uint32 level;
  };

  extent2d extent;
  fbo_buffer test_buffer;
  cspan<tex_att_t> attachments;
};

enum ctx_fbo_status {
  CTX_FBO_STATUS_OK = 0,
  CTX_FBO_STATUS_INVALID_HANDLE,
};

struct icontext {
  virtual ~icontext() = default;
  virtual void get_meta(ctx_meta& meta) = 0;

  virtual ctx_buff_status create_buffer(ctx_buff& buff, const ctx_buff_desc& desc) = 0;
  virtual ctx_buff_status update_buffer(ctx_buff buff, const buffer_data& data) = 0;
  virtual ctx_buff_status map_buffer(ctx_buff buff, void** ptr, size_t size, size_t offset) = 0;
  virtual ctx_buff_status unmap_buffer(ctx_buff buff, void* ptr) noexcept = 0;
  virtual ctx_buff_status destroy_buffer(ctx_buff buff) noexcept = 0;

  virtual ctx_tex_status create_texture(ctx_tex& tex, const ctx_tex_desc& desc) = 0;
  virtual ctx_tex_status update_texture(ctx_tex tex, const ctx_tex_data& data) = 0;
  virtual ctx_tex_status update_texture(ctx_tex tex, const ctx_tex_opts& opts) = 0;
  virtual ctx_tex_status destroy_texture(ctx_tex tex) noexcept = 0;

  virtual ctx_shad_status create_shader(ctx_shad& shad, shad_err_str& err,
                                        const ctx_shad_desc& desc) = 0;
  virtual ctx_shad_status destroy_shader(ctx_shad shad) noexcept = 0;

  virtual ctx_pip_status create_pipeline(ctx_pip& pip, pip_err_str& err,
                                         const ctx_pip_desc& desc) = 0;
  virtual ctx_pip_status destroy_pipeline(ctx_pip pip) noexcept = 0;

  virtual ctx_fbo_status create_framebuffer(ctx_fbo& fbo, const ctx_fbo_desc& desc) = 0;
  virtual ctx_fbo_status destroy_framebuffer(ctx_fbo fbo) noexcept = 0;

  virtual void submit_render_data(context_t ctx, cspan<ctx_render_data> render_data) = 0;

  virtual void device_wait() = 0;
  virtual void swap_buffers() = 0;
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

struct texture_t_ : public ctx_res_node<texture_t_> {
public:
  texture_t_(context_t ctx, ctx_tex handle_, const ctx_tex_desc& desc) noexcept;

public:
  ctx_tex handle;
  std::atomic<uint32> refcount;
  texture_type type;
  image_format format;
  texture_sampler sampler;
  texture_addressing addressing;
  extent3d extent;
  uint32 levels;
  uint32 layers;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(texture_t_);
};

struct buffer_t_ : public ctx_res_node<buffer_t_> {
public:
  buffer_t_(context_t ctx_, ctx_buff handle_, const ctx_buff_desc& desc) noexcept;

public:
  ctx_buff handle;
  buffer_type type;
  buffer_flag flags;
  size_t sie;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(buffer_t_);
};

struct shader_t_ : public ctx_res_node<shader_t_> {
public:
  shader_t_(context_t ctx_, ctx_shad handle_, const ctx_shad_desc& desc) noexcept;

public:
  ctx_shad handle;
  shader_type type;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(shader_t_);
};

struct uniform_t_ {
public:
  uniform_t_(pipeline_t pip_, ctx_unif handle_,
             ctx_alloc::string_t<char> name_, attribute_type type_, size_t size_) noexcept;

public:
  pipeline_t pip;
  ctx_unif handle;
  ctx_alloc::string_t<char> name;
  attribute_type type;
  size_t size;
};

struct pipeline_t_ : public ctx_res_node<pipeline_t_> {
public:
  using unif_map = ctx_alloc::string_fhashmap_t<uniform_t_>;

public:
  pipeline_t_(context_t ctx_, ctx_pip handle_,
              stages_flag stages_, primitive_mode primitive_, polygon_mode poly_mode_,
              ctx_alloc::uarray_t<attribute_binding>&& layout, unif_map&& unifs_) noexcept;

public:
  ctx_pip handle;
  stages_flag stages;
  primitive_mode primitive;
  polygon_mode poly_mode;
  ctx_alloc::uarray_t<attribute_binding> layout;
  unif_map unifs;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(pipeline_t_);
};

struct framebuffer_t_ : public ctx_res_node<framebuffer_t_> {
// public:
  // enum attachment_state {
  //   ATT_NONE = 0,
  //   ATT_TEX,
  //   ATT_BUFF,
  // };

public:
  framebuffer_t_(context_t ctx_,
                 extent2d extent_, fbo_buffer test_buffer_,
                 const ctx_render_data::fbo_data_t& fdata_) noexcept;

  framebuffer_t_(context_t ctx_, ctx_fbo handle_,
                 extent2d extent_, fbo_buffer test_buffer_,
                 ctx_alloc::uarray_t<fbo_image>&& attachments,
                 const ctx_render_data::fbo_data_t& fdata_) noexcept;

  // framebuffer_t_(context_t ctx_, ctx_fbo handle_,
  //                extent2d extent_, fbo_buffer test_buffer_,
  //                image_format color_buffer_,
  //                const ctx_render_data::fbo_data_t& fdata_) noexcept;

public:
  ctx_fbo handle;
  extent2d extent;
  fbo_buffer test_buffer;
  optional<ctx_alloc::uarray_t<fbo_image>> attachments;
  ctx_alloc::vec_t<ctx_render_cmd> cmds;
  ctx_render_data::fbo_data_t fdata;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(framebuffer_t_);
};

struct context_t_ {
public:
  context_t_(ctx_alloc::uptr_t<ctx_alloc>&& alloc,
             ctx_alloc::uptr_t<icontext>&& renderer,
             window_t win, context_api api,
             extent2d fbo_ext, fbo_buffer fbo_tbuff,
             const ctx_render_data::fbo_data_t& fdata) noexcept;

public:
  void insert_node(buffer_t buff);
  void insert_node(texture_t tex);
  void insert_node(shader_t shad);
  void insert_node(pipeline_t pip);
  void insert_node(framebuffer_t fbo);

  void remove_node(buffer_t buff);
  void remove_node(texture_t tex);
  void remove_node(shader_t shad);
  void remove_node(pipeline_t pip);
  void remove_node(framebuffer_t fbo);

public:
  template<typename F>
  void for_each_fbo(F&& fun) {
    framebuffer_t curr = _fbo_list;
    while (curr) {
      fun(curr);
      curr = curr->next;
    }
    fun(&_default_fbo);
  }

public:
  icontext& renderer() { return *_renderer; }
  ctx_alloc& alloc() { return *_alloc; }
  context_api api() const { return _api; }
  framebuffer_t default_fbo() { return &_default_fbo;}
  window_t window() const { return _win; }
  size_t fbo_count() const { return _fbo_list_sz; }

public:
  ctx_alloc::uptr_t<ctx_alloc> on_destroy();

private:
  ctx_alloc::uptr_t<ctx_alloc> _alloc;
  ctx_alloc::uptr_t<icontext> _renderer;

  context_api _api;
  window_t _win;

  framebuffer_t_ _default_fbo;

  size_t _fbo_list_sz;
  buffer_t _buff_list;
  texture_t _tex_list;
  shader_t _shad_list;
  framebuffer_t _fbo_list;
  pipeline_t _pip_list;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(context_t_);
};

} // namespace ntf::render
