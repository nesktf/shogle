#pragma once

#include <ntfstl/allocator.hpp>
#include <ntfstl/expected.hpp>
#include <ntfstl/function.hpp>
#include <ntfstl/memory_pool.hpp>
#include <ntfstl/ptr.hpp>
#include <ntfstl/span.hpp>

#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
#include <ntfstl/logger.hpp>

#include <algorithm>
#include <string_view>
#endif

// TODO: Move these to ntfstl?
namespace ntf {

namespace meta {
template<typename Cont>
concept any_std_cont = requires(Cont a, const Cont b) {
  requires std::regular<Cont>;
  requires std::swappable<Cont>;
  requires std::destructible<typename Cont::value_type>;
  requires std::same_as<typename Cont::reference, typename Cont::value_type&>;
  requires std::same_as<typename Cont::const_reference, const typename Cont::value_type&>;
  requires std::forward_iterator<typename Cont::iterator>;
  requires std::forward_iterator<typename Cont::const_iterator>;
  requires std::signed_integral<typename Cont::difference_type>;
  requires std::same_as<typename Cont::difference_type,
                        typename std::iterator_traits<typename Cont::iterator>::difference_type>;
  requires std::same_as<
    typename Cont::difference_type,
    typename std::iterator_traits<typename Cont::const_iterator>::difference_type>;
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
concept std_cont_of =
  any_std_cont<Cont> && requires() { requires std::same_as<typename Cont::value_type, T>; };

template<typename Cont, typename T, typename... Args>
concept growable_emplace_container_of =
  std_cont_of<Cont, T> && requires(Cont c, T obj, Args&&... args) {
    { c.emplace_back(obj) } -> std::same_as<typename Cont::reference>;
    { c.emplace_back(std::move(obj)) } -> std::same_as<typename Cont::reference>;
    { c.emplace_back(std::forward<Args>(args)...) } -> std::same_as<typename Cont::reference>;
  };

template<typename Cont, typename T>
concept growable_push_container_of = std_cont_of<Cont, T> && requires(Cont c, T obj) {
  { c.push_back(obj) } -> std::same_as<typename Cont::reference>;
  { c.push_back(std::move(obj)) } -> std::same_as<typename Cont::reference>;
};

template<typename Cont>
concept reservable_std_cont = any_std_cont<Cont> && requires(Cont c, typename Cont::size_type sz) {
  { c.reserve(sz) } -> std::same_as<void>;
};

} // namespace meta

} // namespace ntf

namespace shogle {

using namespace ntf::numdefs;

using ntf::span;

template<typename T>
using ref_view = std::reference_wrapper<T>;

template<typename T>
using ptr_view = ntf::weak_ptr<T>;

template<typename T>
using sv_expect = ntf::expected<T, std::string_view>;

template<typename T>
using s_expect = ntf::expected<T, std::string>;

using scratch_arena = ntf::fixed_arena;

template<typename T>
using arena_alloc = ntf::allocator_adaptor<T, scratch_arena>;

template<typename T>
using scratch_vec = std::vector<T, arena_alloc<T>>;

template<typename T>
auto make_scratch_vec(scratch_arena& arena) -> scratch_vec<T> {
  return scratch_vec<T>(arena_alloc<T>{arena});
}

template<typename F>
class scope_end {
public:
  template<typename Func>
  scope_end(Func&& func) : _func(std::forward<Func>(func)), _engaged(true) {}

  ~scope_end() noexcept {
    if (_engaged) {
      invoke();
    }
  }

public:
  void invoke() noexcept { std::invoke(_func); }

  void disengage() noexcept { _engaged = false; }

private:
  F _func;
  bool _engaged;
};

template<typename Func>
scope_end(Func&& func) -> scope_end<Func>;

struct extent2d {
  u32 width, height;
};

template<typename T>
struct square_pos {
  T x, y;
  T width, height;
};

template<typename T>
struct circle_pos {
  T x, y;
  T radius;
};

struct extent3d {
  u32 width, height, depth;
};

struct color4 {
  f32 r, g, b, a;
};

struct color3 {
  f32 r, g, b;
};

constexpr inline u32 VSPAN_TOMBSTONE = std::numeric_limits<u32>::max();

struct vec_span {
  u32 index;
  u32 count;

  template<typename Vec, typename Fun>
  void for_each(Vec& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index + count <= vec.size());
    for (u32 i = index; i < index + count; ++i) {
      f(vec[i]);
    }
  }

  template<typename Vec, typename Fun>
  void for_each(const Vec& vec, Fun&& f) const {
    NTF_ASSERT(index != VSPAN_TOMBSTONE);
    NTF_ASSERT(index + count <= vec.size());
    for (u32 i = index; i < index + count; ++i) {
      f(vec[i]);
    }
  }
};

} // namespace shogle
