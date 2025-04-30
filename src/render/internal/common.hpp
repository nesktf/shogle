#pragma once

#include "../renderer.hpp"
#include "./platform.hpp"

#include "../../stl/arena.hpp"

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

namespace ntf {

class rp_alloc {
private:
  static r_allocator base_alloc;

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

  void free(void* mem) {
    std::invoke(_free, _user_ptr, mem);
  }

  template<typename T>
  T* allocate_uninited(size_t n = 1u) {
    return static_cast<T*>(allocate(sizeof(T), alignof(T)));
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
    this->free(obj);
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
    this->free(_arena.data());
  }

  bool resize_arena(size_t block_sz) {
    void* old = _arena.data();
    if (arena_init(block_sz)) {
      this->free(old);
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

struct r_texture_ {
  r_texture_(r_context ctx_, r_platform_texture handle_, rp_tex_desc&& desc) :
    ctx{ctx_}, handle{handle_},
    refcount{1},
    type{desc.type}, format{desc.format},
    extent{desc.extent},
    levels{desc.levels}, layers{desc.layers},
    addressing{desc.addressing}, sampler{desc.sampler} {}

  r_context ctx;
  r_platform_texture handle;

  std::atomic<uint32> refcount;
  r_texture_type type;
  r_texture_format format;
  extent3d extent;
  uint32 levels;
  uint32 layers;
  r_texture_address addressing;
  r_texture_sampler sampler;
};

struct r_buffer_ {
  r_buffer_(r_context ctx_, r_platform_buffer handle_, rp_buff_desc&& desc) :
    ctx{ctx_}, handle{handle_},
    type{desc.type}, flags{desc.flags}, size{desc.size} {}

  r_context ctx;
  r_platform_buffer handle;

  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;
};

struct r_shader_ {
  r_shader_(r_context ctx_, r_platform_shader handle_, const r_shader_descriptor& desc) :
    ctx{ctx_}, handle{handle_},
    type{desc.type} {}

  r_context ctx;
  r_platform_shader handle;

  r_shader_type type;
};

struct r_uniform_ {
  r_uniform_(r_pipeline pip, r_platform_uniform location_,
             std::string name_, r_attrib_type type_, size_t size_) :
    pipeline{pip}, location{location_},
    name{std::move(name_)}, type{type_}, size{size_} {}

  r_pipeline pipeline;
  r_platform_uniform location;
  std::string name;
  r_attrib_type type;
  size_t size;
};

struct r_pipeline_ {
  r_pipeline_(r_context ctx_, r_platform_pipeline handle_, const r_pipeline_descriptor& desc,
              std::unique_ptr<vertex_layout>&& layout_, uniform_map&& uniforms_,
              r_stages_flag stages_) :
    ctx{ctx_}, handle{handle_},
    stages{stages_},
    primitive{desc.primitive}, poly_mode{desc.poly_mode},
    layout{std::move(layout_)}, uniforms{std::move(uniforms_)} {}

  r_context ctx;
  r_platform_pipeline handle;

  r_stages_flag stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;

  std::unique_ptr<vertex_layout> layout;

  uniform_map uniforms;
};

struct r_framebuffer_ {
  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 std::vector<r_framebuffer_attachment>&& attachments_) :
    ctx{ctx_}, handle{handle_},
    extent{extent_},
    test_buffer{test_buffer_},
    attachments{std::move(attachments_)} {}

  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 r_texture_format color_buffer_) :
    ctx{ctx_}, handle{handle_},
    extent{extent_},
    test_buffer{test_buffer_},
    attachments{color_buffer_} {}

  r_framebuffer_(r_context ctx_) :
    ctx{ctx_}, handle{DEFAULT_FBO_HANDLE},
    attachments{std::vector<r_framebuffer_attachment>{}} {}

  r_context ctx;
  r_platform_fbo handle;

  extent2d extent;
  r_test_buffer test_buffer;
  std::variant<std::vector<r_framebuffer_attachment>, r_texture_format> attachments;
};

struct r_context_ {
public:
  r_context_(r_platform_context& platform, rp_command_map&& map,
             r_window win, r_api api_, rp_alloc&& alloc) noexcept :
    _api{api_}, _win{win}, _platform{platform},
    _draw_lists{std::move(map)},
    _default_fbo{this}, _alloc{std::move(alloc)} {}

public:
  r_platform_context& renderer() { return _platform; }
  rp_alloc& alloc() { return _alloc; }
  rp_command_map& draw_lists() { return _draw_lists; }
  r_api api() const { return _api; }

public:
  r_api _api;
  r_window _win;
  r_platform_context& _platform;

  std::vector<std::unique_ptr<r_buffer_>> _buffers;
  std::vector<std::unique_ptr<r_texture_>> _textures;
  std::vector<std::unique_ptr<r_framebuffer_>> _framebuffers;
  std::vector<std::unique_ptr<r_shader_>> _shaders;
  std::vector<std::unique_ptr<r_pipeline_>> _pipelines;

  rp_command_map _draw_lists;
  r_framebuffer_ _default_fbo;
  rp_alloc _alloc;
};

} // namespace ntf
