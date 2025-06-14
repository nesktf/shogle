#pragma once

#include "./attributes.hpp"

namespace ntf::render {

struct shader_binding {
  buffer_t buffer;
  uint32 binding;
  size_t size;
  size_t offset;
};

struct vertex_binding {
  buffer_t buffer;
  uint32 layout;
};

struct buffer_binding {
  cspan<vertex_binding> vertex;
  buffer_t index;
  cspan<shader_binding> shader;
};

struct uniform_const {
  uniform_t uniform;
  const void* data;
  attribute_type type;
  size_t alignment;
  size_t size;
};

struct render_opts {
  uint32 vertex_count;
  uint32 vertex_offset;
  uint32 index_offset;
  uint32 instances;
};

struct render_cmd {
  framebuffer_t target;
  pipeline_t pipeline;
  buffer_binding buffers;
  cspan<texture_t> textures;
  cspan<uniform_const> consts;
  render_opts opts;
  uint32 sort_group;
  function_view<void(context_t)> render_callback;
};

struct external_state {
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests test;
};

struct external_cmd {
  framebuffer_t target;
  weak_cptr<external_state> state;
  uint32 sort_group;
  function_view<void(context_t, ctx_handle)> render_callback;
};

struct context_params {
  window_t window;
  context_api ctx_api;
  uint32 swap_interval;
  uvec4 fb_viewport;
  clear_flag fb_clear_flags;
  color4 fb_clear_color;
  weak_cptr<malloc_funcs> alloc;
};

expect<context_t> create_context(const context_params& params);
void destroy_context(context_t ctx) noexcept;

void start_frame(context_t ctx);
void end_frame(context_t ctx);
void device_wait(context_t ctx);
void submit_render_command(context_t ctx, const render_cmd& cmd);
void submit_external_command(context_t ctx, const external_cmd& cmd);
window_t get_window(context_t ctx);
context_api get_api(context_t ctx);
cstring_view<char> get_name(context_t ctx);

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rcontext_ops {
  ntfr::context_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::context_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid context handle");
    return ptr;
  }

public:
  operator ntfr::context_t() const noexcept { return _ptr(); }

  void start_frame() const {
    ntfr::start_frame(_ptr());
  }
  void end_frame() const {
    ntfr::end_frame(_ptr());
  }
  void device_wait() const {
    ntfr::device_wait(_ptr());
  }
  void submit_render_command(const ntfr::render_cmd& cmd) const {
    ntfr::submit_render_command(_ptr(), cmd);
  }
  void submit_external_command(const ntfr::external_cmd& cmd) const {
    ntfr::submit_external_command(_ptr(), cmd);
  }

  ntfr::window_t window() const {
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
  context_view(context_t ctx) noexcept :
    _ctx{ctx} {}

public:
  context_t get() const noexcept { return _ctx; }

private:
  context_t _ctx;
};

class context : public ::ntf::impl::rcontext_ops<context> {
private:
  struct deleter_t {
    void operator()(context_t ctx) noexcept {
      ntfr::destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<context_t>, deleter_t>;

public:
  explicit context(context_t ctx) noexcept :
    _ctx{ctx} {}

public:
  static expect<context> create(const context_params& params) {
    return ntfr::create_context(params)
    .transform([](context_t ctx) -> context {
      return context{ctx};
    });
  }

public:
  context_t get() const noexcept { return _ctx.get(); }

public:
  operator context_view() const noexcept { return {get()}; }

private:
  uptr_type _ctx;
};

} // namespace ntf::render
