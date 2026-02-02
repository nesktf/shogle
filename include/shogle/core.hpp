#pragma once

#include <ntfstl/expected.hpp>
#include <ntfstl/function.hpp>
#include <ntfstl/memory.hpp>
#include <ntfstl/ptr.hpp>
#include <ntfstl/span.hpp>
#include <ntfstl/unique_array.hpp>

#ifndef SHOGLE_DISABLE_INTERNAL_LOGS
#include <ntfstl/logger.hpp>
#endif

#define SHOGLE_THROW(thing_) throw thing_

#define SHOGLE_THROW_IF(cond_, thing_) \
  if (cond_) {                         \
    SHOGLE_THROW(thing_);              \
  }

#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

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

using ntf::ptr_view;
using ntf::ref_view;

template<typename T>
using sv_expect = ntf::expected<T, std::string_view>;

template<typename T>
using s_expect = ntf::expected<T, std::string>;

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
struct rectangle_pos {
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

template<typename T>
using dynvec = std::vector<T, ::ntf::mem::default_pool::allocator<T>>;

template<typename T>
using linked_list = std::list<T, ::ntf::mem::default_pool::allocator<T>>;

template<typename K, typename T, typename Hash = std::hash<K>, typename Pred = std::equal_to<K>>
using linked_hashmap =
  std::unordered_map<K, T, Hash, Pred, ::ntf::mem::default_pool::allocator<std::pair<K, T>>>;

template<typename T, typename Hash = std::hash<T>, typename Pred = std::equal_to<T>>
using linked_set = std::unordered_set<T, Hash, Pred, ::ntf::mem::default_pool::allocator<T>>;

template<typename T>
using unique_ptr = std::unique_ptr<T, ::ntf::mem::default_pool::deleter<T>>;

template<typename T>
using unique_array = ::ntf::unique_array<T, ::ntf::mem::default_pool::deleter<T>>;

using scratch_arena = ::ntf::mem::growing_arena;

template<typename T>
using scratch_dynvec = std::vector<T, scratch_arena::allocator<T>>;

template<typename T>
scratch_dynvec<T> make_scratch_dynvec(scratch_arena& arena) {
  return scratch_vec<T>(scratch_arena::allocator<T>{arena});
}

template<typename T>
using scratch_list = std::list<T, scratch_arena::allocator<T>>;

template<typename T>
scratch_list<T> make_scratch_list(scratch_arena& arena) {
  return scratch_list<T>(scratch_arena::allocator<T>{arena});
}

template<typename T>
using scratch_unique = std::unique_ptr<T, scratch_arena::deleter<T>>;

template<typename T, typename... Args>
scratch_unique<T> make_scratch_unique(scratch_arena& arena, Args&&... args) {
  return scratch_unique<T>(arena.construct<T>(std::forward<Args>(args)...));
}

template<typename T>
using scratch_array = ::ntf::unique_array<T, scratch_arena::deleter<T>>;

template<typename T>
scratch_array<T> make_scratch_array(scratch_arena& arena, scratch_arena::size_type n) {
  auto* ptr = arena.construct_n<T>(n);
  return scratch_array<T>(ptr, n);
}

template<typename T>
scratch_array<T> make_scratch_array(scratch_arena& arena, scratch_arena::size_type n,
                                    const T& copy) {
  auto* ptr = arena.construct_n<T>(n, copy);
  return scratch_array<T>(ptr, n);
}

template<typename T>
scratch_array<T> make_scratch_array(ntf::uninitialized_t tag, scratch_arena& arena,
                                    scratch_arena::size_type n) {
  auto* ptr = arena.construct_n<T>(tag, n);
  return scratch_array<T>(ptr, n);
}

} // namespace shogle
