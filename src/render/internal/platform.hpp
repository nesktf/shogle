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
#include "../buffer.hpp"
#include "../framebuffer.hpp"
#include "../texture.hpp"
#include "../pipeline.hpp"

#include "./allocator.hpp"

#include <variant>
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

struct ctx_unif_meta {
  ctx_unif handle;
  ctx_alloc::string_t<char> name;
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

  static constexpr u32 MAX_LAYOUT_NUMBER = 32u; // hack

public:
  ctx_render_cmd(function_view<void(context_t)> on_render_) :
    external{nullopt}, on_render{on_render_} {}
  ctx_render_cmd(function_view<void(context_t, ctx_handle)> on_render_) :
    external{nullopt}, on_render{on_render_} {}

public:
  ctx_pip pip;
  std::array<ctx_buff, MAX_LAYOUT_NUMBER> vbo;
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
  cstring_view<char> source;
};

using shad_err_str = cstring_view<char>;

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

using pip_err_str = cstring_view<char>;

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
  cspan<tex_att_t> ctx_attachments;
  ctx_alloc::uarray_t<fbo_image> attachments;
};

enum ctx_fbo_status {
  CTX_FBO_STATUS_OK = 0,
  CTX_FBO_STATUS_INVALID_HANDLE,
};

struct icontext {
  virtual ~icontext() = default;
  virtual void get_limits(ctx_limits& limits) = 0;
  virtual ctx_alloc::string_t<char> get_name(ctx_alloc& alloc) = 0;
  virtual void get_dfbo_params(extent2d& ext, fbo_buffer& buff, u32& msaa) = 0;

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
  size_t size;

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
  ctx_alloc::uarray_t<fbo_image> attachments;
  ctx_alloc::vec_t<ctx_render_cmd> cmds;
  ctx_render_data::fbo_data_t fdata;

public:
  NTF_DECLARE_NO_MOVE_NO_COPY(framebuffer_t_);
};

struct context_t_ {
public:
  context_t_(ctx_alloc::uptr_t<ctx_alloc>&& alloc,
             ctx_alloc::uptr_t<icontext>&& renderer,
             ctx_alloc::string_t<char>&& renderer_name,
             // window_t win,
             context_api api,
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
  // window_t window() const { return _win; }
  size_t fbo_count() const { return _fbo_list_sz; }
  cstring_view<char> name() const { return _renderer_name; }

public:
  ctx_alloc::uptr_t<ctx_alloc> on_destroy();

private:
  ctx_alloc::uptr_t<ctx_alloc> _alloc;
  ctx_alloc::uptr_t<icontext> _renderer;
  ctx_alloc::string_t<char> _renderer_name;

  context_api _api;
  // window_t _win;

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
