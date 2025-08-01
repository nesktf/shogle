#pragma once

#include <ntfstl/core.hpp>
#include <ntfstl/ptr.hpp>
#include <ntfstl/expected.hpp>
#include <ntfstl/optional.hpp>
#include <ntfstl/function.hpp>
#include <ntfstl/memory_pool.hpp>

namespace ntf::meta {

// TODO: Move these to ntfstl
template<typename Cont>
concept any_std_cont = requires(Cont a, const Cont b) {
  requires std::regular<Cont>;
  requires std::swappable<Cont>;
  requires std::destructible<typename Cont::value_type>;
  requires std::same_as<typename Cont::reference, typename Cont::value_type &>;
  requires std::same_as<typename Cont::const_reference, const typename Cont::value_type &>;
  requires std::forward_iterator<typename Cont::iterator>;
  requires std::forward_iterator<typename Cont::const_iterator>;
  requires std::signed_integral<typename Cont::difference_type>;
  requires std::same_as<typename Cont::difference_type, typename std::iterator_traits<typename
                                                        Cont::iterator>::difference_type>;
  requires std::same_as<typename Cont::difference_type, typename std::iterator_traits<typename
                                                        Cont::const_iterator>::difference_type>;
  { a.begin() } -> std::same_as<typename Cont::iterator>;
  { a.end() } -> std::same_as<typename Cont::iterator>;
  { b.begin() } -> std::same_as<typename Cont::const_iterator>;
  { b.end() } -> std::same_as<typename Cont::const_iterator>;
  { a.cbegin() } -> std::same_as<typename Cont::const_iterator>;
  { a.cend() } -> std::same_as<typename Cont::const_iterator>;
  { a.size() } -> std::same_as<typename Cont::size_type>;
  { a.max_size() } -> std::same_as<typename Cont::size_type>;
  { a.empty() } -> std::same_as<bool>;
};

template<typename Cont, typename T>
concept std_cont = any_std_cont<Cont> && requires() {
  requires std::same_as<typename Cont::value_type, T>;
};

template<typename Cont, typename T, typename... Args>
concept grow_emplace_cont = std_cont<Cont, T> && requires(Cont c, T obj, Args&&... args) {
  { c.emplace_back(obj) } -> std::same_as<typename Cont::reference>;
  { c.emplace_back(std::move(obj)) } -> std::same_as<typename Cont::reference>;
  { c.emplace_back(std::forward<Args>(args)...) } -> std::same_as<typename Cont::reference>;
};

template<typename Cont, typename T>
concept grow_push_cont = std_cont<Cont, T> && requires(Cont c, T obj) {
  { c.push_back(obj) } -> std::same_as<typename Cont::reference>;
  { c.push_back(std::move(obj)) } -> std::same_as<typename Cont::reference>;
};

template<typename Cont>
concept reservable_std_cont = any_std_cont<Cont> && requires(Cont c, typename Cont::size_type sz) {
  { c.reserve(sz) } -> std::same_as<void>;
};

} // namespace ntf::meta

namespace shogle {

using namespace ntf::numdefs;

using string_view = std::string_view;
using ntf::span;
using ntf::weak_ptr;

class render_error : public std::exception {
public:
  enum code_t : u32 {
    no_error = 0,
    unknown_error,
    alloc_failure,
    invalid_handle,
    invalid_offset,
    no_data,
    buff_alloc_failure,
    buff_not_dynamic,
    buff_not_mappable,
    tex_invalid_layer,
    tex_invalid_extent,
    tex_invalid_level,
    tex_invalid_addressing,
    tex_invalid_sampler,
    tex_out_of_limits_extent,
    tex_no_images,
    pip_no_source,
    pip_invalid_stages,
    pip_compilation_failure,
    pip_linking_failure,
    gl_load_failed,
  };

public:
  render_error() noexcept :
    _msg{}, _code{no_error} {}

  render_error(code_t code) noexcept :
    _msg{}, _code{code} {}

  render_error(code_t code, string_view msg) noexcept :
    _msg{msg}, _code{code} {}

public:
  static const char* error_string(code_t ret);

public:
  code_t code() const { return _code; }
  string_view code_str() const { return error_string(_code); }
  const char* what() const noexcept override { return error_string(_code); }

  string_view msg() const { return _msg; }
  bool has_msg() const { return !_msg.empty(); }

private:
  string_view _msg;
  code_t _code;
};

template<typename T>
using render_expect = ::ntf::expected<T, render_error>;

class win_error : public std::exception {
public:
  enum code_t {
    no_error = 0,
    backend_init_failure,
    creation_failure,
  };

public:
  win_error() noexcept :
    _msg{}, _code{no_error} {}

  win_error(code_t code) noexcept :
    _msg{}, _code{code} {}

  win_error(code_t code, string_view msg) noexcept :
    _msg{msg}, _code{code} {}

public:
  static const char* error_string(code_t ret);

public:
  code_t code() const { return _code; }
  string_view code_str() const { return error_string(_code); }
  const char* what() const noexcept override { return error_string(_code); }

  string_view msg() const { return _msg; }
  bool has_msg() const { return !_msg.empty(); }

private:
  string_view _msg;
  code_t _code;
};

template<typename T>
using win_expect = ::ntf::expected<T, win_error>;

using asset_error = ntf::error<void>;

template<typename T>
using asset_expect = ntf::expected<T, asset_error>;

} // namespace shogle
