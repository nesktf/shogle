#pragma once

#include "../core.hpp"
#include <ntfstl/expected.hpp>
#include <ntfstl/allocator.hpp>

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

constexpr uint32 VSPAN_TOMBSTONE = std::numeric_limits<uint32>::max();
struct vec_span {
  uint32 index;
  uint32 count;

  template<typename Vec, typename Fun>
  void for_each(Vec& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index+count <= vec.size());
    for (uint32 i = index; i < index+count; ++i) {
      f(vec[i]);
    }
  }

  template<typename Vec, typename Fun>
  void for_each(const Vec& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index+count <= vec.size());
    for (uint32 i = index; i < index+count; ++i) {
      f(vec[i]);
    }
  }
}; 

} // namespace ntf
