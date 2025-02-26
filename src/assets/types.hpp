#pragma once

#include "../stl/expected.hpp"
#include "../stl/optional.hpp"
#include "../stl/allocator.hpp"

namespace ntf {

using asset_error = error<void>;

template<typename T>
using asset_expected = expected<T, asset_error>;

enum class r_material_type {
  diffuse = 0,
  specular,
};

} // namespace ntf
