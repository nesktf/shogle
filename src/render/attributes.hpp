#pragma once

#include "./common.hpp"

#include "../math/matrix.hpp"

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag, _underlying, _get_ptr) \
static_assert(std::is_trivial_v<_type>, NTF_STRINGIFY(_type) " is not trivial!!!"); \
template<> \
struct attribute_traits<_type> { \
  static constexpr bool is_specialized = true; \
  using underlying_type = _underlying; \
  static constexpr ntfr::attribute_type tag = _tag; \
  static constexpr size_t size = attribute_size(tag); \
  static constexpr uint32 dim = attribute_dim(tag); \
  static const _underlying* value_ptr(const _type& obj) noexcept { \
    return _get_ptr; \
  } \
}

namespace ntf::render {

enum class attribute_type : uint32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

struct attribute_binding {
  attribute_type type;
  uint32 location;
  size_t offset;
  size_t stride;
};

// For uniforms (who cares about double precision matrices??? Use an uniform buffer!!!!!)
using attribute_data = ntf::inplace_trivial<sizeof(mat4), alignof(mat4)>;

} // namespace ntf::render

namespace ntf::meta {

template<typename T>
struct attribute_traits {
  static constexpr bool is_specialized = false;
};

constexpr inline size_t attribute_size(ntfr::attribute_type type) noexcept {
  constexpr size_t int_sz = sizeof(int32);
  constexpr size_t f32_sz = sizeof(f32);
  constexpr size_t f64_sz = sizeof(f64);

  switch (type) {
    case ntfr::attribute_type::i32:   return int_sz;
    case ntfr::attribute_type::ivec2: return 2*int_sz;
    case ntfr::attribute_type::ivec3: return 3*int_sz;
    case ntfr::attribute_type::ivec4: return 4*int_sz;

    case ntfr::attribute_type::f32:   return f32_sz;
    case ntfr::attribute_type::vec2:  return 2*f32_sz;
    case ntfr::attribute_type::vec3:  return 3*f32_sz;
    case ntfr::attribute_type::vec4:  return 4*f32_sz;
    case ntfr::attribute_type::mat3:  return 9*f32_sz;
    case ntfr::attribute_type::mat4:  return 16*f32_sz;

    case ntfr::attribute_type::f64:   return f64_sz;
    case ntfr::attribute_type::dvec2: return 2*f64_sz;
    case ntfr::attribute_type::dvec3: return 3*f64_sz;
    case ntfr::attribute_type::dvec4: return 4*f64_sz;
    case ntfr::attribute_type::dmat3: return 9*f64_sz;
    case ntfr::attribute_type::dmat4: return 16*f64_sz;
  };

  return 0;
};

constexpr inline uint32 attribute_dim(ntfr::attribute_type type) noexcept {
  switch (type) {
    case ntfr::attribute_type::i32:   [[fallthrough]];
    case ntfr::attribute_type::f32:   [[fallthrough]];
    case ntfr::attribute_type::f64:   return 1;

    case ntfr::attribute_type::vec2:  [[fallthrough]];
    case ntfr::attribute_type::ivec2: [[fallthrough]];
    case ntfr::attribute_type::dvec2: return 2;

    case ntfr::attribute_type::vec3:  [[fallthrough]];
    case ntfr::attribute_type::ivec3: [[fallthrough]];
    case ntfr::attribute_type::dvec3: return 3;

    case ntfr::attribute_type::vec4:  [[fallthrough]];
    case ntfr::attribute_type::ivec4: [[fallthrough]];
    case ntfr::attribute_type::dvec4: return 4;

    case ntfr::attribute_type::mat3:  [[fallthrough]];
    case ntfr::attribute_type::dmat3: return 9;

    case ntfr::attribute_type::mat4:  [[fallthrough]];
    case ntfr::attribute_type::dmat4: return 16;
  };

  return 0;
}

SHOGLE_DECLARE_ATTRIB_TRAIT(f32, ntfr::attribute_type::f32, f32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2, ntfr::attribute_type::vec2, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3, ntfr::attribute_type::vec3, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4, ntfr::attribute_type::vec4, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3, ntfr::attribute_type::mat3, f32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4, ntfr::attribute_type::mat4, f32, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(f64, ntfr::attribute_type::f64, f64, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2, ntfr::attribute_type::dvec2, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3, ntfr::attribute_type::dvec3, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4, ntfr::attribute_type::dvec4, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat3, ntfr::attribute_type::dmat3, f64, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(dmat4, ntfr::attribute_type::dmat4, f64, glm::value_ptr(obj));

SHOGLE_DECLARE_ATTRIB_TRAIT(int32, ntfr::attribute_type::i32, int32, &obj);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2, ntfr::attribute_type::ivec2, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3, ntfr::attribute_type::ivec3, int32, glm::value_ptr(obj));
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4, ntfr::attribute_type::ivec4, int32, glm::value_ptr(obj));

template<typename T>
concept attribute_type = attribute_traits<T>::is_specialized;

} // namespace ntf::meta

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
