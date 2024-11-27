#pragma once

#include "../core.hpp"

#include <fmt/format.h>

// To add:
// - std::vector
// - std::list
// - std::queue
// - std::array
// - std::function
// - std::map/std::unordered_map
// - std::string?

namespace ntf {

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
