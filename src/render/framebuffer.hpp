#pragma once

#include "./context.hpp"

namespace ntf {

class r_framebuffer_view {
public:
  r_framebuffer_view(r_framebuffer fbo) noexcept :
    _fbo{fbo} {}

public:
  static r_framebuffer_view default_fbo(r_context_view ctx) {
    return r_framebuffer_view{r_get_default_framebuffer(ctx.handle())};
  }

public:
  void clear_flags(r_clear_flag flags) const {
    r_framebuffer_set_clear(_fbo, flags);
  }
  void clear_color(const color4& color) const {
    r_framebuffer_set_color(_fbo, color);
  }
  void viewport(const uvec4& vp) const {
    r_framebuffer_set_viewport(_fbo, vp);
  }
  
public:
  r_framebuffer handle() const { return _fbo; }
  r_context_view context() const {
    return r_framebuffer_get_ctx(_fbo);
  }

  r_clear_flag clear_flags() const {
    return r_framebuffer_get_clear(_fbo);
  }
  color4 clear_color() const {
    return r_framebuffer_get_color(_fbo);
  }
  uvec4 viewport() const {
    return r_framebuffer_get_viewport(_fbo);
  }

protected:
  r_framebuffer _fbo;
};


class renderer_framebuffer : public r_framebuffer_view {
private:
  struct deleter_t {
    void operator()(r_framebuffer fbo) {
      r_destroy_framebuffer(fbo);
    }
  };
  using uptr_type = std::unique_ptr<r_framebuffer_, deleter_t>;

public:
  explicit renderer_framebuffer(r_framebuffer fbo) noexcept :
    r_framebuffer_view{fbo},
    _handle{fbo} {}

public:
  static auto create(
    r_context_view ctx, const r_framebuffer_descriptor& desc
  ) noexcept -> r_expected<renderer_framebuffer>
  {
    return r_create_framebuffer(ctx.handle(), desc)
      .transform([](r_framebuffer fbo)-> renderer_framebuffer {
        return renderer_framebuffer{fbo};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_framebuffer_descriptor& desc
  ) -> renderer_framebuffer
  {
    return renderer_framebuffer{r_create_framebuffer(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};

} // namespace ntf
