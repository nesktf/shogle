#pragma once

#include "../renderer.hpp"
#include "./platform.hpp"

#include <ntfstl/hashmap.hpp>
#include <atomic>

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

namespace ntf {

template<typename T>
class rp_res_node {
public:
  rp_res_node(r_context ctx_) noexcept :
    ctx{ctx_}, prev{nullptr}, next{nullptr} {}

public:
  r_context ctx;
  T *prev, *next;
};

struct r_texture_ : public rp_res_node<r_texture_> {
public:
  r_texture_(r_context ctx_, r_platform_texture handle_, const rp_tex_desc& desc) noexcept;

public:
  r_platform_texture handle;
  std::atomic<uint32> refcount;
  r_texture_type type;
  r_texture_format format;
  extent3d extent;
  uint32 levels;
  uint32 layers;
  r_texture_address addressing;
  r_texture_sampler sampler;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_texture_);
};

struct r_buffer_ : public rp_res_node<r_buffer_> {
public:
  r_buffer_(r_context ctx_, r_platform_buffer handle_, const rp_buff_desc& desc) noexcept;

public:
  r_platform_buffer handle;
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_buffer_);
};

struct r_shader_ : public rp_res_node<r_shader_> {
public:
  r_shader_(r_context ctx_, r_platform_shader handle_, const rp_shad_desc& desc) noexcept;

public:
  r_platform_shader handle;
  r_shader_type type;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_shader_);
};

struct r_uniform_ {
public:
  r_uniform_(r_pipeline pip, r_platform_uniform location_,
             std::string name_, r_attrib_type type_, size_t size_) noexcept;

public:
  r_pipeline pipeline;
  r_platform_uniform location;
  std::string name;
  r_attrib_type type;
  size_t size;
};

struct r_pipeline_ : public rp_res_node<r_pipeline_> {
public:
  using uniform_map = fixed_hashmap<
    std::string, r_uniform_,
    std::hash<std::string>, std::equal_to<std::string>,
    // rp_alloc::alloc_del_t<std::pair<const std::string, r_uniform_>>
    allocator_delete<std::pair<const std::string, r_uniform_>,
      rp_alloc::adaptor_t<std::pair<const std::string, r_uniform_>>
    >
  >;
public:
  r_pipeline_(r_context ctx_, r_platform_pipeline handle_,
              r_stages_flag stages_, r_primitive primitive_, r_polygon_mode poly_mode_,
              rp_alloc::uarray_t<r_attrib_binding>&& layout_,
              uniform_map&& uniforms_) noexcept;

public:
  r_platform_pipeline handle;
  r_stages_flag stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;
  rp_alloc::uarray_t<r_attrib_binding> layout;
  uniform_map uniforms;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_pipeline_);
};

struct r_framebuffer_ : public rp_res_node<r_framebuffer_> {
public:
  enum attachment_state {
    ATT_NONE = 0,
    ATT_TEX,
    ATT_BUFF,
  };

public:
  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 rp_alloc::uarray_t<r_framebuffer_attachment>&& attachments_,
                 const rp_fbo_frame_data& fdata_) noexcept;

  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 r_texture_format color_buffer_,
                 const rp_fbo_frame_data& fdata_) noexcept;

  r_framebuffer_(r_context ctx_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 const rp_fbo_frame_data& fdata_) noexcept;

public:
  r_platform_fbo handle;
  extent2d extent;
  r_test_buffer test_buffer;
  union {
    rp_alloc::uarray_t<r_framebuffer_attachment> attachments;
    r_texture_format color_buffer;
    char _dummy;
  };
  attachment_state att_state;
  rp_alloc::vec_t<rp_draw_cmd> cmds;
  rp_fbo_frame_data fdata;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_framebuffer_);
};

struct r_context_ {
public:
  r_context_(rp_alloc::uptr_t<rp_alloc>&& alloc,
             rp_alloc::uptr_t<rp_context>&& renderer,
             r_window win, r_api api_,
             extent2d fbo_ext, r_test_buffer fbo_tbuff,
             const rp_fbo_frame_data& fdata) noexcept;

public:
  void insert_node(r_buffer buff);
  void remove_node(r_buffer buff);

  void insert_node(r_texture tex);
  void remove_node(r_texture tex);

  void insert_node(r_framebuffer fbo);
  void remove_node(r_framebuffer fbo);

  void insert_node(r_shader shad);
  void remove_node(r_shader shad);

  void insert_node(r_pipeline pip);
  void remove_node(r_pipeline pip);

public:
  template<typename F>
  void for_each_fbo(F&& fun) {
    r_framebuffer_* curr = _fbo_list;
    while (curr) {
      fun(*curr);
      curr = curr->next;
    }
    fun(_default_fbo);
  }

public:
  size_t fbo_count() const { return _fbo_list_sz; }

public:
  rp_context& renderer() { return *_renderer; }
  rp_alloc& alloc() { return *_alloc; }
  r_api api() const { return _api; }
  r_framebuffer_& default_fbo() { return _default_fbo;}
  r_window window() const { return _win; }

public:
  rp_alloc::uptr_t<rp_alloc> on_destroy();

private:
  rp_alloc::uptr_t<rp_alloc> _alloc;
  rp_alloc::uptr_t<rp_context> _renderer;

  r_api _api;
  r_window _win;

  r_framebuffer_ _default_fbo;

  size_t _fbo_list_sz;
  r_buffer_* _buff_list;
  r_texture_* _tex_list;
  r_shader_* _shad_list;
  r_framebuffer_* _fbo_list;
  r_pipeline_* _pip_list;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_context_);
};

} // namespace ntf
