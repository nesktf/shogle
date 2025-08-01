#pragma once

#include <shogle/render/types.hpp>

namespace shogle {

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
  span<const vertex_binding> vertex;
  buffer_t index;
  span<const shader_binding> shader;
};

struct texture_binding {
  texture_t texture;
  u32 sampler;
};

struct uniform_const {
  attribute_data data;
  attribute_type type;
  u32 location;
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
  span<const texture_binding> textures;
  span<const uniform_const> consts;
  render_opts opts;
  uint32 sort_group;
  ntf::function_view<void(context_t)> render_callback;
};

struct external_state {
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests test;
};

struct external_cmd {
  framebuffer_t target;
  weak_ptr<const external_state> state;
  uint32 sort_group;
  ntf::function_view<void(context_t, ctx_handle)> render_callback;
};

struct context_params {
  const void* ctx_params;
  context_api ctx_api;
  uvec4 fb_viewport;
  clear_flag fb_clear_flags;
  color4 fb_clear_color;
  weak_ptr<const ntf::malloc_funcs> alloc;
};

expect<context_t> create_context(const context_params& params);
void destroy_context(context_t ctx) noexcept;

void start_frame(context_t ctx);
void end_frame(context_t ctx);
void device_wait(context_t ctx);
void submit_render_command(context_t ctx, const render_cmd& cmd);
void submit_external_command(context_t ctx, const external_cmd& cmd);
context_api get_api(context_t ctx);
cstring_view<char> get_name(context_t ctx);

namespace impl {

template<typename Derived>
class rcontext_ops {
  context_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    context_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid context handle");
    return ptr;
  }

public:
  operator context_t() const noexcept { return _ptr(); }

  void start_frame() const {
    ::shogle::start_frame(_ptr());
  }
  void end_frame() const {
    ::shogle::end_frame(_ptr());
  }
  void device_wait() const {
    ::shogle::device_wait(_ptr());
  }
  void submit_render_command(const render_cmd& cmd) const {
    ::shogle::submit_render_command(_ptr(), cmd);
  }
  void submit_external_command(const external_cmd& cmd) const {
    ::shogle::submit_external_command(_ptr(), cmd);
  }

  context_api api() const {
    return ::shogle::get_api(_ptr());
  }
};

} // namespace impl

class context_view : public impl::rcontext_ops<context_view> {
public:
  context_view(context_t ctx) noexcept :
    _ctx{ctx} {}

public:
  context_t get() const noexcept { return _ctx; }

private:
  context_t _ctx;
};

class context : public impl::rcontext_ops<context> {
private:
  struct deleter_t {
    void operator()(context_t ctx) noexcept {
      ::shogle::destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<context_t>, deleter_t>;

public:
  explicit context(context_t ctx) noexcept :
    _ctx{ctx} {}

public:
  static expect<context> create(const context_params& params) {
    return ::shogle::create_context(params)
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

} // namespace shogle
