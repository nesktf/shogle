#pragma once

#include "vector.hpp"

#if SHOGLE_USE_STL_COMPLEX
#include <complex>
#endif

namespace ntf {

#if SHOGLE_USE_STL_COMPLEX
template<typename T>
using complex = std::complex<T>;
#endif

using cmplx = complex<float32>;
using dcmplx = complex<float64>;
using icmplx = complex<int32>;
using ucmplx = complex<uint32>;

constexpr inline float32 norm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr inline cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

constexpr inline vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

constexpr inline cmplx normalize(cmplx z) {
#if SHOGLE_USE_GLM
  return z/glm::sqrt(norm2(z));
#endif
}

constexpr inline float norm(cmplx z) {
#if SHOGLE_USE_GLM
  return glm::sqrt(norm2(z));
#endif
}

constexpr inline cmplx expic(float theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

} // namespace ntf
