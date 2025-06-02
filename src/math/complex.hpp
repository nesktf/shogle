#pragma once

#include "./vector.hpp"

#include <complex>

namespace ntf {

template<typename T>
using complex = std::complex<T>;

using cmplx = complex<f32>;
using dcmplx = complex<f64>;
using icmplx = complex<int32>;
using ucmplx = complex<uint32>;

constexpr inline f32 norm2(cmplx z) {
  return z.real()*z.real() + z.imag()*z.imag();
}

constexpr inline cmplx conv(const vec2& v) {
  return cmplx{v.x, v.y};
}

constexpr inline vec2 conv(const cmplx& z) {
  return vec2{z.real(), z.imag()};
}

constexpr inline cmplx normalize(cmplx z) {
  return z/glm::sqrt(norm2(z));
}

constexpr inline float norm(cmplx z) {
  return glm::sqrt(norm2(z));
}

constexpr inline cmplx expic(float theta) {
  return cmplx{glm::cos(theta), glm::sin(theta)};
}

} // namespace ntf
