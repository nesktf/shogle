#pragma once

#include "../stl/expected.hpp"
#include "../stl/optional.hpp"
#include "../stl/allocator.hpp"

#if defined(SHOGLE_ASSETS_USE_EXCEPTIONS) && SHOGLE_ASSETS_USE_EXCEPTIONS
#define SHOGLE_ASSET_THROW(msg, ...) \
  throw ::ntf::asset_error::format({msg} __VA_OPT__(,) __VA_ARGS__)
#else
#define SHOGLE_ASSET_THROW(msg, ...) NTF_ASSERT(false, msg __VA_OPT__(,) __VA_ARGS__)
#endif

namespace ntf {

using asset_error = error<void>;

template<typename T>
using asset_expected = expected<T, asset_error>;

enum class r_material_type {
  diffuse = 0,
  specular,
};

} // namespace ntf
