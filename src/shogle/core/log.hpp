#pragma once

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <string>
#include <iostream>
#include <chrono>

#define NTF_LOG_ERROR_COL "[0;31m"
#define NTF_LOG_WARNING_COL "[0;33m"
#define NTF_LOG_INFO_COLOR "[0;34m"
#define NTF_LOG_DEBUG_COLOR "[0;32m"
#define NTF_LOG_VERBOSE_COLOR "[0;37m"

namespace ntf {

enum class loglevel {
  error = 0,
  warning,
  info,
  debug,
  verbose,
};

class log {
public:
  inline static void set_level(loglevel new_level) {
    log_level = new_level;
  }

  template<typename... Args>
  inline static void _log(loglevel level, const std::string& prefix, const std::string& str_color, fmt::format_string<Args...> format, Args&&... args) {
    if (log_level < level) {
      return;
    }

#ifdef NTF_LOG_PRINT_TIME
    using TimePoint = std::chrono::system_clock::time_point;
    TimePoint now = std::chrono::system_clock::now();
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    std::tm time_tm = fmt::localtime(std::chrono::system_clock::to_time_t(now));

    std::string log = fmt::format(format, std::forward<Args>(args)...);
    fmt::print("[{:%H:%M:%S}.{:03d}]\033{}[{}]\033[0m{}\n", time_tm, (int)ms, str_color, prefix, log);
#else
    fmt::print("\033{}[{}]\033[0m{}\n", str_color, prefix, fmt::format(format, std::forward<Args>(args)...));
#endif
  }

  template <typename... Args>
  inline static void error(fmt::format_string<Args...> format, Args&&... args) {
    _log(loglevel::error, "E", NTF_LOG_ERROR_COL, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void warning(fmt::format_string<Args...> format, Args&&... args) {
    _log(loglevel::warning, "W", NTF_LOG_WARNING_COL, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void info(fmt::format_string<Args...> format, Args&&... args) {
    _log(loglevel::info, "I", NTF_LOG_INFO_COLOR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void debug(fmt::format_string<Args...> format, Args&&... args) {
    _log(loglevel::debug, "D", NTF_LOG_DEBUG_COLOR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void verbose(fmt::format_string<Args...> format, Args&&... args) {
    _log(loglevel::verbose, "V", NTF_LOG_VERBOSE_COLOR, format, std::forward<Args>(args)...);
  }

private:
  inline static loglevel log_level = loglevel::info;
};

} // namespace ntf
