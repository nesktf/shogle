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
#include <source_location>

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

template<typename T>
concept is_complete = requires(T obj) {
  { sizeof(obj) };
};


template<typename... FmtArgs>
struct error_fmt {
  consteval error_fmt(fmt::format_string<FmtArgs...> fmt_,
                      const std::source_location& loc_ = std::source_location::current()) :
    fmt{fmt_}, loc{loc_} {}

  fmt::format_string<FmtArgs...> fmt;
  std::source_location loc;
};

template<typename T = void>
class error : public std::exception {
public:
  template<typename U>
  error(U&& data, std::string msg,
        const std::source_location& loc = std::source_location::current()) :
    _data{std::forward<U>(data)}, _msg{std::move(msg)}, _loc{loc} {}

public:
  const char* what() const noexcept override { return _msg.c_str();  }
  const std::source_location& where() const noexcept { return _loc; }

  const T& data() const noexcept { return _data; }
  T& data() noexcept { return _data; }

  const std::string& msg() const noexcept { return _msg; }
  std::string& msg() noexcept { return _msg; }

public:
  template<is_forwarding<T> U, typename... Args>
  static error format(U&& data, const error_fmt<Args...>& msg, Args&&... args) {
    return {std::forward<U>(data), fmt::format(msg.fmt, std::forward<Args>(args)...), msg.loc};
  }

protected:
  T _data;
  std::string _msg;
  std::source_location _loc;
};

template<>
class error<void> : public std::exception {
public:
  error(std::string msg, const std::source_location& loc = std::source_location::current()) :
    _msg{std::move(msg)}, _loc{loc} {}

public:
  const char* what() const noexcept override { return _msg.c_str(); }
  const std::source_location& where() const noexcept { return _loc; }

  const std::string& msg() const noexcept { return _msg; }
  std::string& msg() noexcept { return _msg; }

public:
  template<typename... Args>
  static error format(const error_fmt<Args...>& msg, Args&&... args) {
    return {fmt::format(msg.fmt, std::forward<Args>(args)...), msg.loc};
  }

private:
  std::string _msg;
  std::source_location _loc;
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

template<typename T>
class span_view {
public:
  using const_iterator = const T*;

public:
  span_view() noexcept :
    _data{nullptr}, _size{0} {}

  span_view(const T* arr, size_t size) noexcept :
    _data{arr}, _size{size} {}

  span_view(const T& obj) noexcept :
    _data{std::addressof(obj)}, _size{1} {}

  span_view(const T* obj) noexcept :
    _data{obj}, _size{1} {}

  template<size_t N>
  span_view(const T(&arr)[N]) noexcept :
    _data{std::addressof(arr)}, _size{N} {}

  template<typename Alloc>
  span_view(const std::vector<T, Alloc>& vec) noexcept :
    _data{vec.data()}, _size{vec.size()} {}

  template<size_t N>
  span_view(std::array<T, N>& arr) noexcept :
    _data{arr.data()}, _size{N} {}

public:
  const T& operator[](size_t idx) const {
    NTF_ASSERT(idx <= _size);
    return _data[idx];
  }

  const T* data() const noexcept { return _data; }
  [[nodiscard]] size_t size() const noexcept { return _size; }

  bool has_data() const { return _data != nullptr && _size > 0; }
  explicit operator bool() const { return has_data(); }

  const_iterator begin() const { return _data; }
  const_iterator cbegin() const { return _data; }
  const_iterator end() const { return _data+_size;}
  const_iterator cend() const { return _data+_size; }

private:
  const T* _data;
  size_t _size;
};

} // namespace ntf
