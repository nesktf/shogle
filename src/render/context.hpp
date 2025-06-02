#pragma once

#include "./attributes.hpp"

namespace ntf::render {

struct shader_buffer {
  buffer_ptr buffer;
  uint32 binding;
  size_t size;
  size_t offset;
};

struct buffer_binding {
  buffer_ptr vertex;
  buffer_ptr index;
  cspan<shader_buffer> shader;
};

struct uniform_const {
  uniform_ptr uniform;
  const void* data;
  attribute_type type;
  size_t alignment;
  size_t size;
};

struct render_opts {
  uint32 vertex_count;
  uint32 vertex_offset;
  uint32 instances;
};

struct render_command {
  framebuffer_ptr target;
  pipeline_ptr pipeline;
  buffer_binding buffers;
  cspan<texture_ptr> textures;
  cspan<uniform_const> consts;
  render_opts opts;
  uint32 sort_group;
  function_view<void(context_ptr)> render_callback;
};

struct external_state {
  primitive_mode primitive;
  polygon_mode poly_mode;
  optional<f32> poly_width;
  pipeline_tests test;
};

struct external_command {
  framebuffer_ptr target;
  optional<external_state> state;
  uint32 sort_grou;
  function_view<void(context_ptr, internal_handle)> render_callback;
};

struct context_params {
  window_ptr window;
  context_api ctx_api;
  uint32 swap_interval;
  uvec4 fb_viewport;
  clear_flag fb_clear_flags;
  color4 fb_clear_color;
  optional<user_alloc> alloc;
};

expect<context_ptr> create_context(const context_params& params);
void destroy_context(context_ptr ctx) noexcept;

void start_frame(context_ptr ctx);
void end_frame(context_ptr ctx);
void device_wait(context_ptr ctx);
void submit_render_command(context_ptr ctx, const render_command& cmd);
void submit_external_command(context_ptr ctx, const external_command& cmd);
window_ptr get_window(context_ptr ctx);
context_api get_api(context_ptr ctx);

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rcontext_ops {
  ntfr::context_ptr _ptr() const noexcept {
    return static_cast<Derived&>(*this).get();
  }

public:
  operator ntfr::context_ptr() const noexcept { return _ptr(); }

  void start_frame() const {
    ntfr::start_frame(_ptr());
  }
  void end_frame() const {
    ntfr::end_frame(_ptr());
  }
  void device_wait() const {
    ntfr::device_wait(_ptr());
  }
  void submit_render_command(const ntfr::render_command& cmd) const {
    ntfr::submit_render_command(_ptr(), cmd);
  }
  void submit_external_command(const ntfr::external_command& cmd) const {
    ntfr::submit_external_command(_ptr(), cmd);
  }

  ntfr::window_ptr window() const {
    return ntfr::get_window(_ptr());
  }
  ntfr::context_api api() const {
    return ntfr::get_api(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class context_view : public impl::rcontext_ops<context_view> {
public:
  context_view(context_ptr ctx) noexcept :
    _ctx{ctx} {}

public:
  context_ptr get() const noexcept { return _ctx; }

private:
  context_ptr _ctx;
};

class context : public ::ntf::impl::rcontext_ops<context> {
private:
  struct deleter_t {
    void operator()(context_ptr ctx) noexcept {
      ntfr::destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<context_ptr>, deleter_t>;

public:
  explicit context(context_ptr ctx) noexcept :
    _ctx{ctx} {}

public:
  static expect<context> create(const context_params& params) {
    return ntfr::create_context(params)
    .transform([](context_ptr ctx) -> context {
      return context{ctx};
    });
  }

public:
  context_ptr get() const noexcept { return _ctx.get(); }

public:
  operator context_view() const noexcept { return {get()}; }

private:
  uptr_type _ctx;
};

} // namespace ntf::render
