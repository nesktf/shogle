#pragma once

#include "./stl/logger.hpp"

#if defined(NDEBUG)
#define NTF_ABORT() ::std::abort()
#elif defined(__clang__)
#define NTF_ABORT() __builtin_debugtrap()
#else
#define NTF_ABORT() __builtin_trap()
#endif

namespace ntf::impl {

template<typename... Args>
void assert_failure(const char* cond, const char* func, const char* file, int line,
                    const char* msg, Args&&... args) {
  if (msg) {
    ::ntf::logger::fatal(" {}:{}: {} assertion '{}' failed: {}", file, line, func, cond,
                    fmt::vformat(msg, fmt::make_format_args(std::forward<Args>(args)...)));
  } else {
    ::ntf::logger::fatal(" {}:{}: {} assertion '{}' failed", file, line, func, cond);
  }
}

} // namespace ntf::impl

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

