#pragma once

#include "./renderer.hpp"

namespace ntf {

class r_context_view {
public:
  r_context_view(r_context ctx) noexcept :
    _ctx{ctx} {}

public:
  r_context handle() const { return _ctx; }

  void start_frame() const {
    r_start_frame(_ctx);
  }
  void end_frame() const {
    r_end_frame(_ctx);
  }
  void device_wait() const {
    r_device_wait(_ctx);
  }
  void submit_command(const r_draw_command& cmd) const {
    r_submit_command(_ctx, cmd);
  }

protected:
  r_context _ctx;
};

class renderer_context : public r_context_view {
private:
  struct deleter_t {
    void operator()(r_context ctx) {
      r_destroy_context(ctx);
    }
  };
  using uptr_type = std::unique_ptr<r_context_, deleter_t>;

public:
  explicit renderer_context(r_context ctx) noexcept :
    r_context_view{ctx},
    _handle{ctx} {}

public:
  static auto create(
    const r_context_params& params
  ) noexcept -> r_expected<renderer_context>
  {
    return r_create_context(params)
      .transform([](r_context ctx) -> renderer_context {
        return renderer_context{ctx};
      });
  }

private:
  uptr_type _handle;
};

} // namespace ntf
