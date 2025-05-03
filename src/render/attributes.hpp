#pragma once

#include "./renderer.hpp"

#include "../math/matrix.hpp"

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag, _underlying, _get_ptr) \
template<> \
struct r_attrib_traits<_type> { \
  using underlying_type = _underlying; \
  static constexpr r_attrib_type tag = _tag; \
  static constexpr size_t size = r_attrib_type_size(tag); \
  static constexpr uint32 dim = r_attrib_type_dim(tag); \
  static constexpr bool is_attrib = true; \
  static const _underlying* value_ptr(const _type& obj) noexcept { \
    return _get_ptr; \
  } \
}

namespace ntf {

template<typename T>
struct r_attrib_traits {
  static constexpr bool is_attrib = false;
};

template<typename T>
concept renderer_attribute_type = r_attrib_traits<T>::is_attrib;

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

SHOGLE_DECLARE_ATTRIB_TRAIT(float, r_attrib_type::f32, float, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2, r_attrib_type::vec2, float, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3, r_attrib_type::vec3, float, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4, r_attrib_type::vec4, float, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3, r_attrib_type::mat3, float, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4, r_attrib_type::mat4, float, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(double, r_attrib_type::f64, double, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2, r_attrib_type::dvec2, double, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3, r_attrib_type::dvec3, double, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4, r_attrib_type::dvec4, double, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3, r_attrib_type::dmat3, double, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4, r_attrib_type::dmat4, double, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(int32, r_attrib_type::i32, int32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2, r_attrib_type::ivec2, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3, r_attrib_type::ivec3, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4, r_attrib_type::ivec4, int32, glm::value_ptr(obj));

template<renderer_attribute_type T>
constexpr r_push_constant r_format_pushconst(r_uniform uniform, const T& data) {
  return r_push_constant{
    .uniform = uniform,
    .data = {
      .data = r_attrib_traits<T>::value_ptr(data),
      .type = r_attrib_traits<T>::tag,
      .alignment = alignof(T),
      .size = sizeof(T),
    },
  };
}

} // namespace ntf

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
