#pragma once

#include "./buffer.hpp"
#include "./texture.hpp"
#include "./pipeline.hpp"
#include "./framebuffer.hpp"

namespace shogle {

struct context_t_ {
public:
  context_t_(ctx_alloc::uptr_t<ctx_alloc>&& alloc,
             ctx_alloc::uptr_t<icontext>&& renderer,
             ctx_alloc::string_t&& renderer_name,
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
  string_view name() const { return _renderer_name; }

public:
  ctx_alloc::uptr_t<ctx_alloc> on_destroy();

private:
  ctx_alloc::uptr_t<ctx_alloc> _alloc;
  ctx_alloc::uptr_t<icontext> _renderer;
  ctx_alloc::string_t _renderer_name;

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

} // namespace shogle
