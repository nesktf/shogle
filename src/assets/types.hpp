#pragma once

#include "../stl/expected.hpp"
#include "../stl/optional.hpp"
#include "../stl/allocator.hpp"

#if defined(SHOGLE_ASSETS_USE_EXCEPTIONS) && SHOGLE_ASSETS_USE_EXCEPTIONS
#define SHOGLE_ASSET_THROW(msg, ...) \
  throw ::ntf::asset_error::format({msg} __VA_OPT__(,) __VA_ARGS__)
#define SHOGLE_ASSET_NOEXCEPT
#else
#define SHOGLE_ASSET_THROW(msg, ...) NTF_ASSERT(false, msg __VA_OPT__(,) __VA_ARGS__)
#define SHOGLE_ASSET_NOEXCEPT NTF_ASSERT_NOEXCEPT
#endif
#define SHOGLE_ASSET_THROW_IF(cond, ...) \
  if (cond) { SHOGLE_ASSET_THROW(__VA_ARGS__); }

namespace ntf {

using asset_error = error<void>;

template<typename T>
using asset_expected = expected<T, asset_error>;

enum class r_material_type {
  diffuse = 0,
  specular,
};

} // namespace ntf
