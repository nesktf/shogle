#pragma once

// Macro utils
#define NTF_NOOP (void)0
#define NTF_EMPTY_MACRO

#define NTF_FILE __FILE__
#define NTF_LINE __LINE__
#define NTF_FUNC __PRETTY_FUNCTION__

// #define NTF_UNUSED(expr) (void)(true ? NTF_NOOP : ((void)(expr)))

#define NTF_APPLY_VA_ARGS(M, ...) NTF_APPLY_VA_ARGS_(M, (__VA_ARGS__))
#define NTF_APPLY_VA_ARGS_(M, args) M args

#define NTF_JOIN(lhs, rhs) NTF_JOIN_(lhs, rhs)
#define NTF_JOIN_(lhs, rhs) NTF_JOIN__(lhs, rhs)
#define NTF_JOIN__(lhs, rhs) lhs##rhs

#define NTF_NARG_(_0, _1, _2, _3, _4, _5, _6, _7, _8, \
                  _9, _10, _11, _12, _13, _14, _15, _16, \
                  _17, _18, _19, _20, _21, _22, _23, _24, \
                  _25, _26, _27, _28, _29, _30, _31, _32, _33, ...) _33

#define NTF_NARG(...) NTF_APPLY_VA_ARGS(NTF_NARG_, NTF_EMPTY_MACRO, ##__VA_ARGS__, \
                                        32, 31, 30, 29, 28, 27, 26, 25, \
                                        24, 23, 22, 21, 20, 19, 18, 17, \
                                        16, 15, 14, 13, 12, 11, 10, 9, \
                                        8, 7, 6, 5, 4, 3, 2, 1, 0, NTF_EMPTY_MACRO)

// RAII declarations/definitions
#define NTF_DECLARE_MOVE_COPY(__type) \
  ~__type() noexcept; \
  __type(__type&&) noexcept; \
  __type(const __type&) noexcept; \
  __type& operator=(__type&&) noexcept; \
  __type& operator=(const __type&) noexcept

#define NTF_DECLARE_MOVE_ONLY(__type) \
  ~__type() noexcept; \
  __type(__type&&) noexcept; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) noexcept; \
  __type& operator=(const __type&) = delete

#define NTF_DECLARE_COPY_ONLY(__type) \
  ~__type() noexcept; \
  __type(__type&&) = delete; \
  __type(const __type&) noexcept; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) noexcept

#define NTF_DECLARE_NO_MOVE_NO_COPY(__type) \
  ~__type() noexcept; \
  __type(__type&&) = delete; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) = delete

#define NTF_DISABLE_MOVE(__type) \
  __type(__type&&) = delete; \
  __type(const __type&) = default; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) = default

#define NTF_DISABLE_COPY(__type) \
  __type(__type&&) = default; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) = default; \
  __type& operator=(const __type&) = delete

#define NTF_DISABLE_MOVE_COPY(__type) \
  __type(__type&&) = delete; \
  __type(const __type&) = delete; \
  __type& operator=(__type&&) = delete; \
  __type& operator=(const __type&) = delete

#define NTF_DISABLE_NOTHING(__type) \
  __type(__type&&) = default; \
  __type(const __type&) = default; \
  __type& operator=(__type&&) = default; \
  __type& operator=(const __type&) = default


#include "./config.hpp"

// Config dependent macros
#define NTF_INLINE inline __attribute__((always_inline))
#define NTF_CONSTEXPR constexpr

#define NTF_LIKELY(arg) __builtin_expect(!!(arg), !0)
#define NTF_UNLIKELY(arg) __builtin_expect(!!(arg), 0)

#ifdef SHOGLE_ENABLE_INTERNAL_LOGS
#define SHOGLE_LOG(_priority, _fmt, ...) \
  ::ntf::logger::_priority("[ShOGLE]" _fmt, ##__VA_ARGS__)
#else
#define SHOGLE_LOG(_priority, _fmt, ...) NTF_NOOP
#endif

#include <string_view>

#define SHOGLE_STRINGIFY_(a) #a
#define SHOGLE_STRINGIFY(a) SHOGLE_STRINGIFY_(a)

#include "./assert.hpp"
