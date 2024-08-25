#pragma once

#ifndef SHOGLE_ENABLE_INTERNAL_LOGS
#define SHOGLE_ENABLE_INTERNAL_LOGS 1
#endif

#include <array>
#include <vector>
#include <queue>
#include <memory>
#include <functional>
#include <map>
#include <unordered_map>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <functional>

#include <sys/types.h>

#define NTF_NOOP (void)0

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


#include <shogle/core/log.hpp>

#ifdef SHOGLE_ENABLE_INTERNAL_LOGS
#define SHOGLE_INTERNAL_LOG_FMT(__priority, __format, ...) ::ntf::log::__priority(__format, __VA_ARGS__)
#define SHOGLE_INTERNAL_LOG(__priority, __msg, ...) ::ntf::log::__priority(__msg)
#else
#define SHOGLE_INTERNAL_LOG_FMT(__priority, __format, ...) NTF_NOOP
#define SHOGLE_INTERNAL_LOG(__priority, __msg, ...) NTF_NOOP
#endif

#include <shogle/core/error.hpp>
// #include <shogle/math/alg.hpp>

namespace ntf {

template<typename F>
struct cleanup {
  F _f;
  cleanup(F f) : _f(f){}
  ~cleanup() {_f();}
};

template<typename T, typename U>
using pair_vector = std::vector<std::pair<T,U>>;

template<typename T>
using polymorphic_vector = std::vector<std::unique_ptr<T>>;

template<typename T>
using strmap = std::unordered_map<std::string, T>;

template<typename TL, typename... TR>
concept same_as_any = (... or std::same_as<TL, TR>);

} // namespace ntf
