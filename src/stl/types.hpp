#pragma once

#include "../core.hpp"

#include <fmt/format.h>

#include <vector>
#include <exception>
#include <functional>
#include <queue>
#include <memory>
#include <utility>
#include <initializer_list>
#include <algorithm>
#include <functional>
#include <list>
#include <string>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <fstream>
#include <sstream>

#include <cstring>
#include <cstdlib>

namespace ntf {

using size_t    = std::size_t;
using ptrdiff_t = std::ptrdiff_t;
using uintptr_t = std::uintptr_t;

using uint8   = std::uint8_t;
using uint16  = std::uint16_t;
using uint32  = std::uint32_t;
using uint64  = std::uint64_t;
using int8    = std::int8_t;
using int16   = std::int16_t;
using int32   = std::int32_t;
using int64   = std::int64_t;
using float32 = float;
using float64 = double;

class error : public std::exception {
public:
  template<typename... Args>
  error(fmt::format_string<Args...> format, Args&&... args) :
    msg(fmt::format(format, std::forward<Args>(args)...)) {}

public:
  const char* what() const noexcept override {
    return msg.c_str();
  }

protected:
  std::string msg;
};

template<typename TL, typename... TR>
concept same_as_any = (... or std::same_as<TL, TR>);

template<typename T>
concept has_operator_equals = requires(T a, T b) {
  { a == b } -> std::convertible_to<bool>;
};

template<typename T>
concept has_operator_nequals = requires(T a, T b) {
  { a != b } -> std::convertible_to<bool>;
};

template<typename T, typename U>
concept is_forwarding = std::is_same_v<U, std::remove_cvref_t<T>>;

template<typename T>
concept not_void = !std::is_void_v<T>;

template<typename T>
concept is_nothrow_forward_constructible = 
  (std::is_rvalue_reference_v<T> && std::is_nothrow_move_constructible_v<T>) ||
  (std::is_lvalue_reference_v<T> && std::is_nothrow_copy_constructible_v<T>);

} // namespace ntf
