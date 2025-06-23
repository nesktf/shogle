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

struct pipeline_desc {
  cspan<attribute_binding> attributes;
  cspan<shader_t> stages;
  primitive_mode primitive;
  polygon_mode poly_mode;
  f32 poly_width;
  render_tests tests;
};

expect<pipeline_t> create_pipeline(context_t ctx, const pipeline_desc& desc);
void destroy_pipeline(pipeline_t pipeline) noexcept;

span<uniform_t> pipeline_get_uniforms(pipeline_t pipeline);
uniform_t pipeline_get_uniform(pipeline_t pip, cstring_view<char> name);
size_t pipeline_get_uniform_count(pipeline_t pip);
attribute_type uniform_get_type(uniform_t uniform);
cstring_view<char> uniform_get_name(uniform_t uniform);
u32 uniform_get_location(uniform_t uniform);

stages_flag pipeline_get_stages(pipeline_t pipeline);
context_t pipeline_get_ctx(pipeline_t pipeline);
ctx_handle pipeline_get_id(pipeline_t pipeline);

class uniform_view {
public:
  constexpr uniform_view() noexcept :
    _unif{nullptr} {}

  constexpr uniform_view(uniform_t unif) noexcept :
    _unif{unif} {}

public:
  attribute_type type() const {
    return ntfr::uniform_get_type(_assert_get());
  }

  std::string_view name() const {
    return ntfr::uniform_get_name(_assert_get());
  }

  u32 location() const {
    return ntfr::uniform_get_location(_assert_get());
  }

private:
  ntfr::uniform_t _assert_get() const noexcept(NTF_ASSERT_NOEXCEPT) {
    NTF_ASSERT(_unif, "Invalid uniform handle");
    return _unif;
  }

public:
  operator ntfr::uniform_t() const { return _assert_get(); }

  ntfr::uniform_t get() const noexcept { return _unif; }

  bool empty() const noexcept {return _unif == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uniform_t _unif;
};

template<meta::attribute_type T>
constexpr uniform_const format_uniform_const(uniform_view uniform, const T& data) {
  return {
    .data = meta::attribute_traits<T>::value_ptr(data),
    .type = meta::attribute_traits<T>::tag,
    .alignment = alignof(T),
    .size = sizeof(T),
    .location = uniform.location(),
  };
}

template<meta::attribute_type T>
constexpr uniform_const format_uniform_const(u32 location, const T& data){
  return {
    .data = meta::attribute_traits<T>::value_ptr(data),
    .type = meta::attribute_traits<T>::tag,
    .alignment = alignof(T),
    .size = sizeof(T),
    .location = location,
  };
}

} // namespace ntf::render

namespace ntf::impl {

template<typename Derived>
class rpipeline_ops {
  ntfr::pipeline_t _ptr() const noexcept(NTF_ASSERT_NOEXCEPT) {
    ntfr::pipeline_t ptr = static_cast<const Derived&>(*this).get();
    NTF_ASSERT(ptr, "Invalid pipeline handle");
    return ptr;
  }

public:
  operator ntfr::pipeline_t() const noexcept(NTF_ASSERT_NOEXCEPT) { return _ptr(); }

  ntfr::context_view context() const {
    return {ntfr::pipeline_get_ctx(_ptr())};
  }
  ntfr::stages_flag stages() const {
    return ntfr::pipeline_get_stages(_ptr());
  }
  ntfr::uniform_view uniform(ntfr::cstring_view<char> name) const {
    return {ntfr::pipeline_get_uniform(_ptr(), name)};
  }
  size_t uniform_count() const {
    return ntfr::pipeline_get_uniform_count(_ptr());
  }
  cspan<ntfr::uniform_t> uniforms() const {
    return ntfr::pipeline_get_uniforms(_ptr());
  }
  ntfr::ctx_handle id() const {
    return ntfr::pipeline_get_id(_ptr());
  }
};

} // namespace ntf::impl

namespace ntf::render {

class pipeline_view : public impl::rpipeline_ops<pipeline_view> {
public:
  pipeline_view() noexcept :
    _pip{nullptr} {}

  pipeline_view(pipeline_t pip) noexcept :
    _pip{pip} {}

public:
  pipeline_t get() const noexcept { return _pip;}

  bool empty() const noexcept {return _pip == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  pipeline_t _pip;
};

class pipeline : public impl::rpipeline_ops<pipeline> {
private:
  struct deleter_t {
    void operator()(pipeline_t pip) noexcept {
      ntfr::destroy_pipeline(pip);
    }
  };
  using uptr_type = std::unique_ptr<std::remove_pointer_t<ntfr::pipeline_t>, deleter_t>;

public:
  pipeline() noexcept :
    _pip{nullptr} {}

  explicit pipeline(pipeline_t pip) noexcept :
    _pip{pip} {}

public:
  static expect<pipeline> create(context_view ctx, const pipeline_desc& desc){
    return ntfr::create_pipeline(ctx.get(), desc)
    .transform([](pipeline_t pip) -> pipeline {
      return pipeline{pip};
    });
  }

public:
  pipeline_t get() const noexcept { return _pip.get(); }
  [[nodiscard]] pipeline_t release() noexcept { return _pip.release(); }

  bool empty() const noexcept { return _pip.get() == nullptr; }
  explicit operator bool() const noexcept { return !empty(); }

private:
  uptr_type _pip;
};

} // namespace ntf
