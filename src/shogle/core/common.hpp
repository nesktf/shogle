#pragma once

#define NTF_NOOP (void)0
#define NTF_UNUSED(expr) (void)(true ? NTF_NOOP : ((void)(expr)))
#define NTF_EMPTY_MACRO

#define NTF_FILE __FILE__
#define NTF_LINE __LINE__
#define NTF_FUNC __PRETTY_FUNCTION__

#define NTF_INLINE inline __attribute__((always_inline))

#define NTF_LIKELY(arg) __builtin_expect(!!(arg), !0)
#define NTF_UNLIKELY(arg) __builtin_expect(!!(arg), 0)

#if defined(NDEBUG)
#define NTF_ABORT() ::std::abort()
#elif defined(__clang__)
#define NTF_ABORT() __builtin_debugtrap()
#else
#define NTF_ABORT() __builtin_trap()
#endif

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

#define NTF_ASSERT_1(cond, ...) NTF_ASSERT_3(cond, nullptr)
#define NTF_ASSERT_2(cond, msg, ...) NTF_ASSERT_3(cond, msg)
#define NTF_ASSERT_3(cond, msg, ...) \
if (!NTF_UNLIKELY(cond)) { \
  ::ntf::impl::assert_failure(#cond, NTF_FUNC, NTF_FILE, NTF_LINE, msg, ##__VA_ARGS__); \
  NTF_ABORT();\
  __builtin_unreachable(); \
}

#define NTF_ASSERT_(...) NTF_APPLY_VA_ARGS( \
                           NTF_JOIN(NTF_ASSERT_, NTF_NARG(__VA_ARGS__)), __VA_ARGS__)

#define NTF_UNREACHABLE() {\
  NTF_ASSERT(0, "Triggered unreachable code!!!!!");\
  __builtin_unreachable();\
}

#ifdef __has_builtin
#if __has_builtin(__builtin_assume)
#define NTF_ASSUME_(x) __builtin_assume(x)
#endif
#endif

#if !defined(NTF_ASSUME_)
#define NTF_ASSUME_(x) do { if(!NTF_UNLIKELY(x)) { NTF_UNREACHABLE(); } } while(0)
#endif

#ifdef NDEBUG
#define NTF_ASSERT(...) NTF_NOOP
#define NTF_ASSUME(x) NTF_ASSUME_(x)
#else
#define NTF_ASSERT(...) NTF_ASSERT_(__VA_ARGS__)
#define NTF_ASSUME(x) NTF_ASSERT(x)
#endif

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
