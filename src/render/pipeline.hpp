#pragma once

#include "./context.hpp"

namespace ntf {

class r_pipeline_view {
public:
  r_pipeline_view(r_pipeline pip) noexcept :
    _pip{pip} {}

public:
  r_pipeline handle() const { return _pip; }
  r_context_view context() const { return r_pipeline_get_ctx(_pip); }

  r_stages_flag stages() const {
    return r_pipeline_get_stages(_pip);
  }
  optional<r_uniform> uniform(std::string_view name) const {
    return r_pipeline_get_uniform(_pip, name);
  }
  r_uniform uniform(unchecked_t, std::string_view name) const {
    return r_pipeline_get_uniform(::ntf::unchecked, _pip, name);
  }

protected:
  r_pipeline _pip;
};

class renderer_pipeline : public r_pipeline_view {
private:
  struct deleter_t {
    void operator()(r_pipeline pip) {
      r_destroy_pipeline(pip);
    }
  };
  using uptr_type = std::unique_ptr<r_pipeline_, deleter_t>;

public:
  explicit renderer_pipeline(r_pipeline pip) noexcept :
    r_pipeline_view{pip},
    _handle{pip} {}

public:
  static auto create(
    r_context_view ctx, const r_pipeline_descriptor& desc
  ) noexcept -> r_expected<renderer_pipeline>
  {
    return r_create_pipeline(ctx.handle(), desc)
      .transform([](r_pipeline pip) -> renderer_pipeline {
        return renderer_pipeline{pip};
      });
  }

  static auto create(
    unchecked_t,
    r_context_view ctx, const r_pipeline_descriptor& desc
  ) -> renderer_pipeline
  {
    return renderer_pipeline{r_create_pipeline(::ntf::unchecked, ctx.handle(), desc)};
  }

private:
  uptr_type _handle;
};

} // namespace ntf
