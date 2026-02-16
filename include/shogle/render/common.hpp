#pragma once

#include <shogle/core.hpp>

#include <shogle/util/logger.hpp>
#include <shogle/util/memory.hpp>

#include <shogle/math/matrix4x4.hpp>
#include <shogle/math/vector2.hpp>
#include <shogle/math/vector3.hpp>
#include <shogle/math/vector4.hpp>

#include <array>

namespace shogle {

enum class render_context_tag {
  none = 0,
  opengl,
  vulkan,
  software,
};

// clang-format off

enum class attribute_type : u32 {
  f32 = 0, vec2,  vec3,  vec4,  mat3,  mat4,
  f64, dvec2, dvec3, dvec4, /* dmat3, dmat4, */
  i32, ivec2, ivec3, ivec4,
	u32, uvec2, uvec3, uvec4,
};

constexpr inline u32 ATTRIBUTE_COUNT = static_cast<u32>(attribute_type::uvec4)+1;

namespace meta {

template<typename T>
struct renderer_traits {
	static constexpr bool is_specialized = false;
};

template<typename T>
concept renderer_context_type = renderer_traits<T>::is_specialized;

template<typename T>
concept renderer_object_type = requires() {
	requires renderer_context_type<typename T::context_type>;
};

template<typename T>
struct extent_traits {
	static constexpr bool is_specialized = false;
};

template<typename T>
concept extent_type = extent_traits<T>::is_specialized;

template<>
struct extent_traits<u32> {
	static constexpr bool is_specialized = true;
	static constexpr u32 dimension_count = 1u;
	static constexpr extent3d offset_cast3d(u32 ext) { return {ext, 0, 0}; }
	static constexpr extent3d extent_clamp3d(u32 ext) {
		return {std::max(ext, 1u), 1u, 1u};
  }
  template<typename T>
  static constexpr size_t stride_of(u32 ext) noexcept {
    return ext*sizeof(T);
  }
};

template<>
struct extent_traits<extent2d> {
	static constexpr bool is_specialized = true;
	static constexpr u32 dimension_count = 2u;
	static constexpr extent3d offset_cast3d(const extent2d& ext) {
		return {ext.width, ext.height, 0};
  }
	static constexpr extent3d extent_clamp3d(const extent2d& ext) {
		return {std::max(ext.width, 1u), std::max(ext.height, 1u), 1u};
  }
  template<typename T>
  static constexpr size_t stride_of(const extent2d& ext) noexcept {
    return ext.height*ext.width*sizeof(T);
  }
};

template<>
struct extent_traits<extent3d> {
	static constexpr bool is_specialized = true;
	static constexpr u32 dimension_count = 3u;
	static constexpr extent3d offset_cast3d(const extent3d& ext) { return ext; }
	static constexpr extent3d extent_clamp3d(const extent3d& ext) {
		return {std::max(ext.width, 1u), std::max(ext.height, 1u), std::max(ext.width, 1u)};
  }
  template<typename T>
  static constexpr size_t stride_of(const extent3d& ext) noexcept {
    return ext.width*ext.height*ext.depth*sizeof(T);
  }
};


template<typename T>
struct attribute_traits {
  static constexpr bool is_specialized = false;
};

template<typename T>
concept attribute_type = attribute_traits<T>::is_specialized;

constexpr inline size_t attribute_size(::shogle::attribute_type type) noexcept {
	constexpr auto size_arr = std::to_array<size_t>({
		sizeof(f32), 2*sizeof(f32), 3*sizeof(f32), 4*sizeof(f64), 9*sizeof(f32), 16*sizeof(f32),
		sizeof(f64), 2*sizeof(f64), 3*sizeof(f64), 4*sizeof(f64), /*9*sizeof(f64), 16*sizeof(f64),*/
		sizeof(i32), 2*sizeof(i32), 3*sizeof(i32), 4*sizeof(i32),
		sizeof(u32), 2*sizeof(u32), 3*sizeof(u32), 4*sizeof(u32),
  });
	static_assert(size_arr.size() == ATTRIBUTE_COUNT);
	const u32 idx = static_cast<u32>(type);
	return idx > ATTRIBUTE_COUNT ? 0 : size_arr[idx];
};

constexpr inline u32 attribute_dim(::shogle::attribute_type type) noexcept {
	constexpr auto dim_arr = std::to_array<u32>({
		1, 2, 3, 4, 9, 16,
		1, 2, 3, 4, /*9, 16,*/
		1, 2, 3, 4,
		1, 2, 3, 4,
  });
	static_assert(dim_arr.size() == ATTRIBUTE_COUNT);
	const u32 idx = static_cast<u32>(type);
	return idx > ATTRIBUTE_COUNT ? 0 : dim_arr[idx];
}

constexpr inline std::string_view attribute_name(::shogle::attribute_type type) noexcept {
  constexpr auto name_arr = std::to_array<const char*>({
		"f32",  "vec2",  "vec3",  "vec4", "mat3", "mat4",
		"f64", "dvec2", "dvec3", "dvec4",
		"i32", "ivec2", "ivec3", "ivec4",
		"u32", "uvec2", "uvec3", "uvec4",
  });
	static_assert(name_arr.size() == ATTRIBUTE_COUNT);
	const u32 idx = static_cast<u32>(type);
	return idx > ATTRIBUTE_COUNT ? "Unknown" : name_arr[idx];
};

#define SHOGLE_DECLARE_ATTRIB_TRAIT(type_, tag_, underlying_) \
static_assert(std::is_trivial_v<type_>, SHOGLE_STRINGIFY(type_) " is not trivial!!!"); \
template<> \
struct attribute_traits<type_> { \
  using underlying_type = underlying_; \
  static constexpr bool is_specialized = true; \
  static constexpr ::shogle::attribute_type tag = tag_;	\
  static constexpr size_t size = attribute_size(tag); \
  static constexpr u32 dimensions = attribute_dim(tag); \
}

SHOGLE_DECLARE_ATTRIB_TRAIT(f32, shogle::attribute_type::f32, f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec2, shogle::attribute_type::vec2, f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec3, shogle::attribute_type::vec3, f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(vec4, shogle::attribute_type::vec4, f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat3, shogle::attribute_type::mat3, f32);
SHOGLE_DECLARE_ATTRIB_TRAIT(mat4, shogle::attribute_type::mat4, f32);

SHOGLE_DECLARE_ATTRIB_TRAIT(f64, shogle::attribute_type::f64, f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec2, shogle::attribute_type::dvec2, f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec3, shogle::attribute_type::dvec3, f64);
SHOGLE_DECLARE_ATTRIB_TRAIT(dvec4, shogle::attribute_type::dvec4, f64);
// SHOGLE_DECLARE_ATTRIB_TRAIT(math::dmat3, shogle::attribute_type::dmat3, f64);
// SHOGLE_DECLARE_ATTRIB_TRAIT(math::dmat4, shogle::attribute_type::dmat4, f64);

SHOGLE_DECLARE_ATTRIB_TRAIT(i32, shogle::attribute_type::i32, i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec2, shogle::attribute_type::ivec2, i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec3, shogle::attribute_type::ivec3, i32);
SHOGLE_DECLARE_ATTRIB_TRAIT(ivec4, shogle::attribute_type::ivec4, i32);

SHOGLE_DECLARE_ATTRIB_TRAIT(u32, shogle::attribute_type::u32, u32);
SHOGLE_DECLARE_ATTRIB_TRAIT(uvec2, shogle::attribute_type::uvec2, u32);
SHOGLE_DECLARE_ATTRIB_TRAIT(uvec3, shogle::attribute_type::uvec3, u32);
SHOGLE_DECLARE_ATTRIB_TRAIT(uvec4, shogle::attribute_type::uvec4, u32);

#undef SHOGLE_DECLARE_ATTRIB_TRAIT

#ifdef SHOGLE_DISABLE_INTERNAL_LOGS
#define SHOGLE_RENDER_LOG(...)
#else
#define SHOGLE_RENDER_LOG(priority_, section_, fmt_, ...)				 \
  ::shogle::logger::log_##priority_(section_, "[{}:{}] " fmt_, 			\
													 ::shogle::meta::render_parse_src_str(__FILE__),	\
                           __LINE__ __VA_OPT__(,) __VA_ARGS__)

consteval std::string_view render_parse_src_str(std::string_view file_str) {
  const char pos[] = {"render/"};
  std::string_view pos_str{&pos[0], 7};
  auto iter = std::search(file_str.begin(), file_str.end(), pos_str.begin(), pos_str.end());
  auto prefix_len = iter - std::begin(file_str) + 7;
  auto out_len = file_str.size() - prefix_len;
  return {file_str.data() + prefix_len, out_len};
}

#endif

// clang-format on

} // namespace meta

struct vertex_attribute {
  u32 location;
  attribute_type type;
  size_t offset;
};

namespace meta {

template<typename T>
concept vertex_type = requires() {
  requires std::convertible_to<std::decay_t<decltype(T::attribute_count)>, u32>;
  requires(T::attribute_count > 0u);
  { T::attributes() } -> std::same_as<std::array<vertex_attribute, T::attribute_count>>;
};

} // namespace meta

template<meta::vertex_type T>
struct soa_vertex_arg {
  static constexpr size_t vertex_stride = 0u;
  using vertex_type = T;
};

template<meta::vertex_type T>
struct aos_vertex_arg {
  static constexpr size_t vertex_stride = sizeof(T);
  using vertex_type = T;
};

} // namespace shogle
