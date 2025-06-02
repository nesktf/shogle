#pragma once

#include "./shader.hpp"

namespace ntf::render {

enum class stages_flag : uint8 {
  none                = 0,
  vertex              = 1 << static_cast<uint8>(shader_type::vertex),
  fragment            = 1 << static_cast<uint8>(shader_type::fragment),
  geometry            = 1 << static_cast<uint8>(shader_type::geometry),
  tesselation_eval    = 1 << static_cast<uint8>(shader_type::tesselation_eval),
  tesselation_control = 1 << static_cast<uint8>(shader_type::tesselation_control),
  compute             = 1 << static_cast<uint8>(shader_type::compute),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(stages_flag);

constexpr stages_flag MIN_RENDER_STAGES = stages_flag::vertex | stages_flag::fragment;
constexpr stages_flag MIN_COMPUTE_STAGES = stages_flag::compute;

struct pipeline_desriptor {
  cspan<attribute_binding> attributes;
  cspan<shader_ptr> stages;
  primitive_mode primitive;
  polygon_mode poly_mode;
  optional<f32> poly_width;
  pipeline_tests tests;
};

expect<pipeline_ptr> create_pipeline(context_ptr ctx, const pipeline_desriptor& desc);
void destroy_pipeline(pipeline_ptr pipeline) noexcept;

span<uniform_ptr> pipeline_get_uniforms(pipeline_ptr pipeline);
uniform_ptr pipeline_get_uniform(pipeline_ptr pip, std::string_view name);
size_t pipeline_get_uniform_count(pipeline_ptr pip);
attribute_type uniform_get_type(uniform_ptr uniform);
std::string_view uniform_get_name(uniform_ptr uniform);

stages_flag pipeline_get_stages(pipeline_ptr pipeline);
context_ptr pipeline_get_ctx(pipeline_ptr pipeline);

class uniform_view {
public:
  constexpr uniform_view(uniform_ptr unif) noexcept :
    _unif{unif} {}

public:
  attribute_type type() const {
    return ntfr::uniform_get_type(_assert_get());
  }

  std::string_view name() const {
    return ntfr::uniform_get_name(_assert_get());
  }

private:
  ntfr::uniform_ptr _assert_get() const noexcept(NTF_ASSERT_NOEXCEPT) {
    NTF_ASSERT(_unif, "Invalid uniform handle");
    return _unif;
  }

public:
  operator ntfr::uniform_ptr() const { return _assert_get(); }

  ntfr::uniform_ptr get() const noexcept { return _unif; }

  bool empty() const noexcept {return _unif == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uniform_ptr _unif;
};

template<meta::attribute_type T>
constexpr uniform_const format_uniform_const(uniform_view uniform, const T& data) {
  return {
    .uniform = uniform.get(),
    .data = meta::attribute_traits<T>::value_ptr(data),
    .type = meta::attribute_traits<T>::tag,
    .alignment = alignof(T),
    .size = sizeof(T),
  };
}

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rpipeline_ops {
  ntfr::pipeline_ptr _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::pipeline_ptr ptr = static_cast<Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid pipeline handle");
    return ptr;
  }

public:
  operator ntfr::pipeline_ptr() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::context_view context() const {
    return {ntfr::pipeline_get_ctx(_ptr())};
  }
  ntfr::stages_flag stages() const {
    return ntfr::pipeline_get_stages(_ptr());
  }
  ntfr::uniform_view uniform(std::string_view name) const {
    return {ntfr::pipeline_get_uniform(_ptr(), name)};
  }
  size_t uniform_count() const {
    return ntfr::pipeline_get_uniform_count(_ptr());
  }
  cspan<ntfr::uniform_ptr> uniforms() const {
    return ntfr::pipeline_get_uniforms(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class pipeline_view : public impl::rpipeline_ops<pipeline_view> {
public:
  pipeline_view(pipeline_ptr pip) noexcept :
    _pip{pip} {}

public:
  pipeline_ptr get() const noexcept { return _pip;}

  bool empty() const noexcept {return _pip == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  pipeline_ptr _pip;
};

class pipeline : public impl::rpipeline_ops<pipeline> {
private:
  struct deleter_t {
    void operator()(pipeline_ptr pip) noexcept {
      ntfr::destroy_pipeline(pip);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::pipeline_ptr>, deleter_t>;

public:
  explicit pipeline(pipeline_ptr pip) noexcept :
    _pip{pip} {}

public:
  static expect<pipeline> create(context_view ctx, const pipeline_desriptor& desc){
    return ntfr::create_pipeline(ctx.get(), desc)
    .transform([](pipeline_ptr pip) -> pipeline {
      return pipeline{pip};
    });
  }

public:
  pipeline_ptr get() const noexcept { return _pip.get(); }
  [[nodiscard]] pipeline_ptr release() noexcept { return _pip.release(); }

  bool empty() const noexcept { return _pip.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _pip;
};

} // namespace ntf
