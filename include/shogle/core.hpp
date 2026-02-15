#pragma once

#include <shogle/config.hpp>

#define SHOGLE_NOOP         (void)0
#define SHOGLE_UNUSED(expr) (void)(true ? SHOGLE_NOOP : ((void)(expr)))

#define SHOGLE_APPLY_VA_ARGS(M, ...)   SHOGLE_APPLY_VA_ARGS_(M, (__VA_ARGS__))
#define SHOGLE_APPLY_VA_ARGS_(M, args) M args

#define SHOGLE_JOIN(lhs, rhs)   SHOGLE_JOIN_(lhs, rhs)
#define SHOGLE_JOIN_(lhs, rhs)  SHOGLE_JOIN__(lhs, rhs)
#define SHOGLE_JOIN__(lhs, rhs) lhs##rhs

#define SHOGLE_NARG_(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                     _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
                     _32, _33, ...)                                                             \
  _33

#define SHOGLE_EMPTY_MACRO
#define SHOGLE_NARG(...)                                                                         \
  SHOGLE_APPLY_VA_ARGS(SHOGLE_NARG_, SHOGLE_EMPTY_MACRO, ##__VA_ARGS__, 32, 31, 30, 29, 28, 27,  \
                       26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, \
                       7, 6, 5, 4, 3, 2, 1, 0, SHOGLE_EMPTY_MACRO)

#define SHOGLE_LIKELY(arg)   __builtin_expect(!!(arg), !0)
#define SHOGLE_UNLIKELY(arg) __builtin_expect(!!(arg), 0)

#define SHOGLE_STRINGIFY_(a) #a
#define SHOGLE_STRINGIFY(a)  SHOGLE_STRINGIFY_(a)

#ifdef NDEBUG
#define SHOGLE_ABORT() ::std::abort()
#else
#ifdef __clang__
#define SHOGLE_ABORT() __builtin_debugtrap()
#else
#define SHOGLE_ABORT() __builtin_trap()
#endif
#endif

#ifdef NDEBUG
#define SHOGLE_ASSERT(...) SHOGLE_NOOP
#else
#define SHOGLE_ASSERT_1(cond, ...) SHOGLE_ASSERT_2(cond, nullptr)
#define SHOGLE_ASSERT_2(cond, msg)                                                         \
  do {                                                                                     \
    if (SHOGLE_UNLIKELY(!std::is_constant_evaluated() && !(cond))) {                       \
      ::shogle::impl::assert_failure(#cond, __PRETTY_FUNCTION__, __FILE__, __LINE__, msg); \
    }                                                                                      \
  } while (0)

#define SHOGLE_ASSERT_(...) \
  SHOGLE_APPLY_VA_ARGS(SHOGLE_JOIN(SHOGLE_ASSERT_, SHOGLE_NARG(__VA_ARGS__)), __VA_ARGS__)

namespace shogle::impl {

[[noreturn]] void assert_failure(const char* cond, const char* func, const char* file, int line,
                                 const char* msg);

}; // namespace shogle::impl

#define SHOGLE_ASSERT(...) SHOGLE_ASSERT_(__VA_ARGS__)
#endif

#define SHOGLE_UNREACHABLE() __builtin_unreachable()

#ifndef SHOGLE_DISABLE_EXCEPTIONS
#define SHOGLE_THROW(thing_) throw thing_

#define SHOGLE_THROW_IF(cond_, thing_) \
  if (cond_) {                         \
    SHOGLE_THROW(thing_);              \
  }
#define SHOGLE_RETHROW() throw
#else
#define SHOGLE_RETHROW()  SHOGLE_NOOP
#define SHOGLE_THROW(err) SHOGLE_ASSERT(false, "Thrown exception " #err)
#define SHOGLE_THROW_IF(cond, err) \
  SHOGLE_ASSERT(false && "Condition " #cond " failed, thrown " #err)
#endif

#if defined(_MSC_VER)
#define SHOGLE_INLINE    __forceinline
#define SHOGLE_NO_INLINE __declspec((noinline))
#elif defined(__GNUC__) || defined(__MINGW32__)
#define SHOGLE_INLINE    inline __attribute__((__always_inline__))
#define SHOGLE_NO_INLINE __attribute__((__noinline__))
#elif defined(__clang__)
#define SHOGLE_INLINE    __forceinline__
#define SHOGLE_NO_INLINE __noinline__
#else
#define SHOGLE_INLINE inline
#define SHOGLE_NO_INLINE
#endif

// clang shits itself with self referential static constexpr member constants for some reason
#if defined(__clang__)
#define SHOGLE_SELF_STATIC_CONST_MEMBER static const inline
#else
#define SHOGLE_SELF_STATIC_CONST_MEMBER static constexpr
#endif

#include <cstdint>
#include <functional>
#include <limits>
#include <utility>

namespace shogle {

namespace numdefs {

using size_t = std::size_t;
using ptrdiff_t = std::ptrdiff_t;
using uintptr_t = std::uintptr_t;

using uint8 = std::uint8_t;
using u8 = uint8;

using uint16 = std::uint16_t;
using u16 = uint16;

using uint32 = std::uint32_t;
using u32 = uint32;

using uint64 = std::uint64_t;
using u64 = uint64;

using int8 = std::int8_t;
using i8 = int8;

using int16 = std::int16_t;
using i16 = int16;

using int32 = std::int32_t;
using i32 = int32;

using int64 = std::int64_t;
using i64 = int64;

using float32 = float;
using f32 = float32;

using float64 = double;
using f64 = float64;

} // namespace numdefs

using namespace numdefs;

using std::in_place_t;
constexpr in_place_t in_place;

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
};

} // namespace shogle
