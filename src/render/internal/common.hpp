#pragma once

#include "../renderer.hpp"
#include "./platform.hpp"

#include "../../stl/hashmap.hpp"

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
  r_texture_(r_context ctx_, r_platform_texture handle_, const rp_tex_desc& desc);
    // :
    // rp_res_node<r_texture_>{ctx_},
    // handle{handle_},
    // refcount{1},
    // type{desc.type}, format{desc.format},
    // extent{desc.extent},
    // levels{desc.levels}, layers{desc.layers},
    // addressing{desc.addressing}, sampler{desc.sampler} {}

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
};

struct r_buffer_ : public rp_res_node<r_buffer_> {
public:
  r_buffer_(r_context ctx_, r_platform_buffer handle_, const rp_buff_desc& desc);
    // :
    // rp_res_node<r_buffer_>{ctx_},
    // handle{handle_},
    // type{desc.type}, flags{desc.flags}, size{desc.size} {}

public:
  r_platform_buffer handle;
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;
};

struct r_shader_ : public rp_res_node<r_shader_> {
public:
  r_shader_(r_context ctx_, r_platform_shader handle_, const rp_shad_desc& desc);
    // :
    // rp_res_node<r_shader_>{ctx_},
    // handle{handle_},
    // type{desc.type} {}

public:
  r_platform_shader handle;
  r_shader_type type;
};

struct r_uniform_ {
public:
  r_uniform_(r_pipeline pip, r_platform_uniform location_,
             std::string name_, r_attrib_type type_, size_t size_);
    // :
    // pipeline{pip}, location{location_},
    // name{std::move(name_)}, type{type_}, size{size_} {}

public:
  r_pipeline pipeline;
  r_platform_uniform location;
  std::string name;
  r_attrib_type type;
  size_t size;
};

struct r_pipeline_ : public rp_res_node<r_pipeline_> {
  r_pipeline_(r_context ctx_, r_platform_pipeline handle_, const rp_pip_desc& desc) ;
    // :
    // rp_res_node<r_pipeline_>{ctx_},
    // handle{handle_},
    // stages{desc.stages_flags},
    // primitive{desc.primitive}, poly_mode{desc.poly_mode},
    // layout{desc.layout}, uniforms{std::move(*desc.uniforms)} {}

public:
  r_platform_pipeline handle;
  r_stages_flag stages;
  r_primitive primitive;
  r_polygon_mode poly_mode;
  vertex_layout* layout;
  fixed_hashmap<std::string, r_uniform_> uniforms;

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
                 span<r_framebuffer_attachment> attachments_,
                 const rp_fbo_frame_data& fdata_);
    // :
    // rp_res_node<r_framebuffer_>{ctx_},
    // handle{handle_},
    // extent{extent_},
    // test_buffer{test_buffer_},
    // attachments{attachments_}, att_state{ATT_TEX} {}

  r_framebuffer_(r_context ctx_, r_platform_fbo handle_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 r_texture_format color_buffer_,
                 const rp_fbo_frame_data& fdata_);
    // :
    // rp_res_node<r_framebuffer_>{ctx_},
    // handle{handle_},
    // extent{extent_},
    // test_buffer{test_buffer_},
    // color_buffer{color_buffer_}, att_state{ATT_BUFF} {}

  r_framebuffer_(r_context ctx_,
                 extent2d extent_, r_test_buffer test_buffer_,
                 const rp_fbo_frame_data& f_data_);
    // :
    // rp_res_node<r_framebuffer_>{ctx_},
    // handle{DEFAULT_FBO_HANDLE},
    // extent{extent_},
    // test_buffer{test_buffer_},
    // _dummy{}, att_state{ATT_NONE},
    // cmds{rp_alloc::adaptor_t<rp_draw_cmd>{ctx->alloc()}}{}

public:
  r_platform_fbo handle;
  extent2d extent;
  r_test_buffer test_buffer;
  union {
    span<r_framebuffer_attachment> attachments;
    r_texture_format color_buffer;
    char _dummy;
  };
  attachment_state att_state;
  std::vector<rp_draw_cmd, rp_alloc::adaptor_t<rp_draw_cmd>> cmds;
  rp_fbo_frame_data fdata;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_framebuffer_);
};

struct r_context_ {
public:
  r_context_(r_window win, r_api api_, rp_alloc&& alloc, r_platform_context& platform,
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
    fun(_default_fbo);
    r_framebuffer_* curr = _fbo_list;
    while (curr) {
      fun(*curr);
      curr = curr->next;
    }
  }

public:
  size_t fbo_count() const { return _fbo_list_sz; }

public:
  r_platform_context& renderer() { return _platform; }
  rp_alloc& alloc() { return _alloc; }
  r_api api() const { return _api; }
  r_framebuffer_& default_fbo() { return _default_fbo;}

private:
  r_api _api;
  r_window _win;
  r_platform_context& _platform;

  r_framebuffer_ _default_fbo;
  rp_alloc _alloc;

  r_buffer_* _buff_list;
  r_texture_* _tex_list;
  r_shader_* _shad_list;
  r_framebuffer_* _fbo_list;
  size_t _fbo_list_sz;
  r_pipeline_* _pip_list;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(r_context_);
};

} // namespace ntf
