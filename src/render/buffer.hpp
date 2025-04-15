#pragma once

#include "../stl/ptr.hpp"
#include "../stl/optional.hpp"

#include "../math/vector.hpp"
#include "../math/matrix.hpp"

#define SHOGLE_DECLARE_ATTRIB_TRAIT(_type, _tag) \
template<> \
struct r_attrib_traits<_type> { \
  static constexpr auto tag = _tag; \
  static constexpr size_t size = r_attrib_type_size(tag); \
  static constexpr uint32 dim = r_attrib_type_dim(tag); \
  static constexpr bool is_attrib = true; \
}

namespace ntf {

NTF_DECLARE_OPAQUE_HANDLE(r_buffer);

enum class r_buffer_type : uint8 {
  vertex = 0,
  index,
  texel,
  uniform,
  shader_storage,
};

enum class r_buffer_flag : uint8 {
  none              = 0,
  dynamic_storage   = 1 << 0,
  read_mappable     = 1 << 1,
  write_mappable    = 1 << 2,
  rw_mappable       = (1<<1) | (1<<2),
};
NTF_DEFINE_ENUM_CLASS_FLAG_OPS(r_buffer_flag);

struct r_buffer_data {
  const void* data;
  size_t size;
  size_t offset;
};

struct r_buffer_descriptor {
  r_buffer_type type;
  r_buffer_flag flags;
  size_t size;

  weak_ref<r_buffer_data> data;
};

struct r_buffer_binding {
  r_buffer buffer;
  r_buffer_type type;
  optional<uint32> location;
};

enum class r_attrib_type : uint32 {
  f32, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, dmat3, dmat4,
  i32, ivec2, ivec3, ivec4,
};

struct r_attrib_binding {
  uint32 binding;
  size_t stride;
};

struct r_attrib_descriptor {
  r_attrib_type type;
  uint32 binding;
  uint32 location;
  size_t offset;
};

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

} // namespace ntf

#undef SHOGLE_DECLARE_ATTRIB_TRAIT
