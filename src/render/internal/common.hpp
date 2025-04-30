#pragma once

#include "../context.hpp"
#include "./platform.hpp"

#include "../../stl/arena.hpp"

#define RET_ERROR(_log_pfx, _fmt, ...) \
  SHOGLE_LOG(error, _log_pfx " " _fmt __VA_OPT__(,) __VA_ARGS__); \
  return unexpected{r_error::format({_fmt} __VA_OPT__(,) __VA_ARGS__)}

#define RET_ERROR_IF(_cond, _log_pfx, _fmt, ...) \
  if (_cond) { \
    RET_ERROR(_log_pfx, _fmt, __VA_ARGS__); \
  }

#define RET_ERROR_CATCH(_log_pfx, _msg) \
  catch (r_error& err) { \
    SHOGLE_LOG(error, _log_pfx " " _msg ": {}", err.what()); \
    return unexpected{std::move(err)}; \
  } catch (const std::exception& ex) { \
    SHOGLE_LOG(error, _log_pfx " " _msg ": {}", ex.what()); \
    return unexpected{r_error::format({"{}"}, ex.what())}; \
  } catch (...) { \
    SHOGLE_LOG(error, _log_pfx " " _msg ": Caught (...)"); \
    return unexpected{r_error{"Caught (...)"}}; \
  }

namespace ntf {

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
  r_context_(std::unique_ptr<r_platform_context>&& platform_, command_map&& map,
             win_handle win_, renderer_api api_, linked_arena&& arena,
             const r_allocator& alloc) noexcept :
    api{api_}, win{win_}, platform{std::move(platform_)},
    draw_lists{std::move(map)}, d_cmd{}, alloc{alloc},
    default_fbo{this}, frame_arena{std::move(arena)} {}

  renderer_api api;
  win_handle win;
  std::unique_ptr<r_platform_context> platform;

  handle_map<r_platform_buffer, r_buffer_> buffers;
  handle_map<r_platform_texture, r_texture_> textures;
  handle_map<r_platform_fbo, r_framebuffer_> framebuffers;
  handle_map<r_platform_shader, r_shader_> shaders;
  handle_map<r_platform_pipeline, r_pipeline_> pipelines;

  command_map draw_lists;
  weak_ref<draw_list> d_list;
  draw_command d_cmd;

  r_allocator alloc;
  r_framebuffer_ default_fbo;
  linked_arena frame_arena;
};

} // namespace ntf
