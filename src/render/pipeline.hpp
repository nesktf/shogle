#pragma once

#include "./renderer.hpp"

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag) \
template<> \
struct r_attrib_traits<_type> { \
  static constexpr auto tag = _tag; \
  static constexpr size_t size = r_attrib_type_size(tag); \
  static constexpr uint32 dim = r_attrib_type_dim(tag); \
  static constexpr bool is_attrib = true; \
}

namespace ntf {

constexpr r_stages_flag r_required_render_stages = r_stages_flag::vertex | r_stages_flag::fragment;

template<typename T>
struct r_attrib_traits {
  static constexpr bool is_attrib = false;
};

template<typename T>
concept shader_attribute_type = r_attrib_traits<T>::is_attrib;

constexpr inline size_t r_attrib_type_size(r_attrib_type type) {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t float_sz = sizeof(float);
  constexpr size_t double_sz = sizeof(double);

  switch (type) {
    case r_attrib_type::i32:   return int_sz;
    case r_attrib_type::ivec2: return 2*int_sz;
    case r_attrib_type::ivec3: return 3*int_sz;
    case r_attrib_type::ivec4: return 4*int_sz;

    case r_attrib_type::f32:   return float_sz;
    case r_attrib_type::vec2:  return 2*float_sz;
    case r_attrib_type::vec3:  return 3*float_sz;
    case r_attrib_type::vec4:  return 4*float_sz;
    case r_attrib_type::mat3:  return 9*float_sz;
    case r_attrib_type::mat4:  return 16*float_sz;

    case r_attrib_type::f64:   return double_sz;
    case r_attrib_type::dvec2: return 2*double_sz;
    case r_attrib_type::dvec3: return 3*double_sz;
    case r_attrib_type::dvec4: return 4*double_sz;
    case r_attrib_type::dmat3: return 9*double_sz;
    case r_attrib_type::dmat4: return 16*double_sz;
  };

  return 0;
};

constexpr inline uint32 r_attrib_type_dim(r_attrib_type type) {
  switch (type) {
    case r_attrib_type::i32:   [[fallthrough]];
    case r_attrib_type::f32:   [[fallthrough]];
    case r_attrib_type::f64:   return 1;

    case r_attrib_type::vec2:  [[fallthrough]];
    case r_attrib_type::ivec2: [[fallthrough]];
    case r_attrib_type::dvec2: return 2;

    case r_attrib_type::vec3:  [[fallthrough]];
    case r_attrib_type::ivec3: [[fallthrough]];
    case r_attrib_type::dvec3: return 3;

    case r_attrib_type::vec4:  [[fallthrough]];
    case r_attrib_type::ivec4: [[fallthrough]];
    case r_attrib_type::dvec4: return 4;

    case r_attrib_type::mat3:  [[fallthrough]];
    case r_attrib_type::dmat3: return 9;

    case r_attrib_type::mat4:  [[fallthrough]];
    case r_attrib_type::dmat4: return 16;
  };

  return 0;
}

SHOGLE_DECLARE_ATTRIB_TRAIT(float32,  r_attrib_type::f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2,     r_attrib_type::vec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3,     r_attrib_type::vec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4,     r_attrib_type::vec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3,     r_attrib_type::mat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4,     r_attrib_type::mat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(float64,  r_attrib_type::f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2,    r_attrib_type::dvec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3,    r_attrib_type::dvec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4,    r_attrib_type::dvec4);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3,    r_attrib_type::dmat3);
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4,    r_attrib_type::dmat4);

SHOGLE_DECLARE_ATTRIB_TRAIT(int32,    r_attrib_type::i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2,    r_attrib_type::ivec2);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3,    r_attrib_type::ivec3);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4,    r_attrib_type::ivec4);

template<shader_attribute_type T>
constexpr r_push_constant r_format_pushconst(r_uniform location, const T& data) {
  return r_push_constant{
    .location = location,
    .data = std::addressof(data),
    .type = r_attrib_traits<T>::tag,
    .alignment = alignof(T),
  };
}

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

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
