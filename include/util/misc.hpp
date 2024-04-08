#pragma once

#include <utility>

namespace ntf {

template<typename T, typename... Args>
inline T* make_ptr(Args&&... args) {
  return new T{std::forward<Args>(args)...};
}

} // namespace ntf::util
