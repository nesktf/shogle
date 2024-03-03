#pragma once

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <string>
#include <iostream>
#include <chrono>

namespace ntf::shogle::logger {

enum class LogLevel {
  LOG_ERROR   = 0,
  LOG_WARNING = 1,
  LOG_INFO    = 2,
  LOG_DEBUG   = 3,
  LOG_VERBOSE = 4
};

#define ERROR_COLOR "[0;31m"
#define WARNING_COLOR "[0;33m"
#define INFO_COLOR "[0;34m"
#define DEBUG_COLOR "[0;32m"
#define VERBOSE_COLOR "[0;37m"

extern LogLevel log_level;

void set_level(LogLevel level);

template<typename... Args>
inline void log(LogLevel level, const std::string& prefix, const std::string& color, fmt::format_string<Args...> format, Args&&... args) {
  if (log_level < level)
    return;

  using TimePoint = std::chrono::system_clock::time_point;

  TimePoint now = std::chrono::system_clock::now();
  uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
  
  std::tm time_tm = fmt::localtime(std::chrono::system_clock::to_time_t(now));

  std::string log = fmt::format(format, std::forward<Args>(args)...);
  fmt::print("{:%H:%M:%S}.{:03d}\033{}[{}]\033[0m {}\n", time_tm, (int)ms, color, prefix, log);
}

template <typename... Args>
inline void fatal(fmt::format_string<Args...> format, Args&&... args) {
  log(LogLevel::LOG_INFO, "ERROR", ERROR_COLOR, format, std::forward<Args>(args)...);
  std::exit(1);
}

template <typename... Args>
inline void error(fmt::format_string<Args...> format, Args&&... args) {
  log(LogLevel::LOG_INFO, "ERROR", ERROR_COLOR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warning(fmt::format_string<Args...> format, Args&&... args) {
  log(LogLevel::LOG_WARNING, "WARNING", WARNING_COLOR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void debug(fmt::format_string<Args...> format, Args&&... args) {
  log(LogLevel::LOG_WARNING, "DEBUG", DEBUG_COLOR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(fmt::format_string<Args...> format, Args&&... args) {
  log(LogLevel::LOG_WARNING, "INFO", INFO_COLOR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void verbose(fmt::format_string<Args...> format, Args&&... args) {
  log(LogLevel::LOG_WARNING, "VERBOSE", VERBOSE_COLOR, format, std::forward<Args>(args)...);
}

}
