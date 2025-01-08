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
#include <variant>
#include <fstream>
#include <sstream>

#include <cstring>
#include <cstdlib>

#define NTF_DECLARE_TAG_TYPE(_name) \
struct _name##_t {}; \
constexpr _name##_t _name{}

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

template<typename T = void>
class error : public std::exception {
public:
  template<typename... Args>
  error(T data, fmt::format_string<Args...> format, Args&&... args)
    noexcept(std::is_nothrow_move_constructible_v<T>) :
    _data{std::move(data)}, _msg{fmt::format(format, std::forward<Args>(args)...)} {}

public:
  const char* what() const noexcept override { return _msg.c_str();  }
  const std::string& msg() const noexcept { return _msg; }
  std::string& msg() noexcept { return _msg; }
  const T& data() const noexcept { return _data; }
  T& data() noexcept { return _data; }

protected:
  T _data;
  std::string _msg;
};

template<>
class error<void> : public std::exception {
public:
  template<typename... Args>
  error(fmt::format_string<Args...> format, Args&&... args) noexcept :
    _msg{fmt::format(format, std::forward<Args>(args)...)} {}

public:
  const char* what() const noexcept override { return _msg.c_str();  }
  const std::string& msg() const noexcept { return _msg; }
  std::string& msg() noexcept { return _msg; }

protected:
  std::string _msg;
};

template<typename T>
class weak_ref {
public:
  constexpr weak_ref() noexcept : _ptr{nullptr} {}
  constexpr weak_ref(T* obj) noexcept : _ptr{obj} {}
  constexpr weak_ref(T& obj) noexcept : _ptr{std::addressof(obj)} {}

public:
  constexpr void reset() { _ptr = nullptr; }
  constexpr void reset(T& obj) { _ptr = std::addressof(obj); }

public:
  constexpr const T* operator->() const { NTF_ASSERT(_ptr); return _ptr; }
  constexpr T* operator->() { NTF_ASSERT(_ptr); return _ptr; }

  constexpr const T& operator*() const { NTF_ASSERT(_ptr); return *_ptr; }
  constexpr T& operator*() { NTF_ASSERT(_ptr); return *_ptr; }

  constexpr const T& get() const { return **this; }
  constexpr T& get() { return **this; }

  [[nodiscard]] constexpr bool valid() const { return _ptr != nullptr; }
  constexpr explicit operator bool() const { return valid(); }

private:
  T* _ptr;
};

template<typename... Fs>
struct visitor_overload : Fs... { using Fs::operator()...; };

template<typename... Fs>
visitor_overload(Fs...) -> visitor_overload<Fs...>;

template<typename... Ts, typename... Fs>
constexpr decltype(auto) operator|(std::variant<Ts...>& v,
                                   const visitor_overload<Fs...>& overload) {
  return std::visit(overload, v);
}

template<typename... Ts, typename... Fs>
constexpr decltype(auto) operator|(const std::variant<Ts...>& v,
                                   const visitor_overload<Fs...>& overload) {
  return std::visit(overload, v);
}

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
