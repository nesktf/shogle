#pragma once

#include "./context.hpp"

namespace ntf {

constexpr r_stages_flag r_required_render_stages = r_stages_flag::vertex | r_stages_flag::fragment;

class r_shader_view {
public:
  r_shader_view(r_shader shader) noexcept :
    _shader{shader} {}

public:
  r_shader handle() const { return _shader; }
  r_context_view context() const {
    return r_shader_get_ctx(_shader);
  }

  r_shader_type type() const {
    return r_shader_get_type(_shader);
  }

protected:
  r_shader _shader;
};

class renderer_shader : public r_shader_view {
private:
  struct deleter_t {
    void operator()(r_shader shader) {
      r_destroy_shader(shader);
    }
  };
  using uptr_type = std::unique_ptr<r_shader_, deleter_t>;

public:
  explicit renderer_shader(r_shader shader) noexcept :
    r_shader_view{shader},
    _handle{shader} {}

public:
  static auto create(
    r_context_view ctx, const r_shader_descriptor& desc
  ) -> r_expected<renderer_shader>
  {
    return r_create_shader(ctx.handle(), desc)
      .transform([](r_shader shad) -> renderer_shader {
        return renderer_shader{shad};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_shader_descriptor& desc
  ) -> renderer_shader
  {
    return renderer_shader{r_create_shader(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};




} // namespace ntf
