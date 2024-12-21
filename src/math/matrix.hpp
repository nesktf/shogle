#pragma once

#include "vector.hpp"

#if SHOGLE_USE_GLM
#include <glm/gtc/matrix_transform.hpp>
#endif

namespace ntf {

#if SHOGLE_USE_GLM
template<uint32 N, uint32 M, typename T>
using mat = glm::mat<N, M, T>;
#endif

using mat3 = mat<3, 3, float32>;
using mat4 = mat<4, 4, float32>;

using dmat3 = mat<3, 3, float64>;
using dmat4 = mat<4, 4, float64>;

} // namespace ntf
