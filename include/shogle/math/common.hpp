#pragma once

#include <shogle/core.hpp>

#include <complex>

#define SHOGLE_MATH_DECL constexpr
#define SHOGLE_MATH_DEF  constexpr SHOGLE_INLINE

#define SHOGLE_MATH_DECLARE_VECTOR_SPECIAL_MEMBERS(type_)      \
  constexpr type_() noexcept = default;                        \
  constexpr type_(const type_&) noexcept = default;            \
  constexpr type_(type_&&) noexcept = default;                 \
  constexpr type_& operator=(const type_&) noexcept = default; \
  constexpr type_& operator=(type_&&) noexcept = default

namespace shogle::math {

template<typename T>
concept numeric_type = std::integral<T> || std::floating_point<T>;

template<typename From, typename To>
concept numeric_convertible = numeric_type<From> && std::convertible_to<From, To>;

template<numeric_type T>
static constexpr T pi = static_cast<T>(3.14159265358979323846);

template<numeric_type T>
static constexpr T e = static_cast<T>(2.7182818284590452354);

template<std::floating_point T>
static constexpr T epsilon = std::numeric_limits<T>::epsilon();

template<numeric_type T>
SHOGLE_MATH_DECL T sqrt(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T rsqrt(T x) noexcept;

SHOGLE_MATH_DECL f32 qrsqrt(float x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T cos(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T sin(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T tan(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T acos(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T asin(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T atan(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T atan2(T y, T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T rad(T degs) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T deg(T rads) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T epsilon_err(T a, T b) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T abs(T x) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T clamp(T x, T min, T max) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T max(T a, T b) noexcept;

template<numeric_type T>
SHOGLE_MATH_DECL T min(T a, T b) noexcept;

template<numeric_type TL, numeric_type TR>
SHOGLE_MATH_DECL auto periodic_add(const TL& a, const TR& b, decltype(a + b) min,
                                   decltype(a + b) max) noexcept;

template<std::floating_point T>
SHOGLE_MATH_DECL bool fequal(T a, T b) noexcept;

} // namespace shogle::math

namespace shogle {

// Forward declarations for vector types
template<u32 N, math::numeric_type T>
struct numvec;

template<u32 N, u32 M, math::numeric_type T>
struct nummat;

template<math::numeric_type T>
struct numquat;

using cmplx = std::complex<f32>;
using cxf32 = std::complex<f32>;
using dcmplx = std::complex<f64>;
using cxf64 = std::complex<f64>;

using vec2 = numvec<2, f32>;
using v2f32 = numvec<2, f32>;
using vec3 = numvec<3, f32>;
using v3f32 = numvec<3, f32>;
using vec4 = numvec<4, f32>;
using v4f32 = numvec<4, f32>;

using dvec2 = numvec<2, f64>;
using v2f64 = numvec<2, f64>;
using dvec3 = numvec<3, f64>;
using v3f64 = numvec<3, f64>;
using dvec4 = numvec<4, f64>;
using v4f64 = numvec<4, f64>;

using ivec2 = numvec<2, i32>;
using v2i32 = numvec<2, i32>;
using ivec3 = numvec<3, i32>;
using v3i32 = numvec<3, i32>;
using ivec4 = numvec<4, i32>;
using v4i64 = numvec<4, i32>;

using uvec2 = numvec<2, u32>;
using v2u32 = numvec<2, u32>;
using uvec3 = numvec<3, u32>;
using v3u32 = numvec<3, u32>;
using uvec4 = numvec<4, u32>;
using v4u64 = numvec<4, u32>;

using mat4 = nummat<4, 4, f32>;
using m4f32 = nummat<4, 4, f32>;
using mat3 = nummat<3, 3, f32>;
using m3f32 = nummat<3, 3, f32>;

using dmat4 = nummat<4, 4, f64>;
using m4f64 = nummat<4, 4, f64>;
using dmat3 = nummat<3, 3, f64>;
using m3f64 = nummat<3, 3, f64>;

using quat = numquat<f32>;
using qf32 = numquat<f32>;
using dquat = numquat<f64>;
using qf64 = numquat<f64>;

} // namespace shogle

#ifndef SHOGLE_MATH_COMMON_INL
#include <shogle/math/common.inl>
#endif
