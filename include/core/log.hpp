/*
    Copyright 2019 natinusala
    Copyright 2019 p-sam

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <string>
#include <iostream>
#include <chrono>

namespace ntf {

#define ERROR_COLOR "[0;31m"
#define WARNING_COLOR "[0;33m"
#define INFO_COLOR "[0;34m"
#define DEBUG_COLOR "[0;32m"
#define VERBOSE_COLOR "[0;37m"

enum class LogLevel {
  LOG_ERROR   = 0,
  LOG_WARNING = 1,
  LOG_INFO    = 2,
  LOG_DEBUG   = 3,
  LOG_VERBOSE = 4
};

class Log {
  using TimePoint = std::chrono::system_clock::time_point;

public:
  inline static void set_level(LogLevel new_level) {
    log_level = new_level;
  }

  template<typename... Args>
  inline static void log(LogLevel level, const std::string& prefix, const std::string& str_color, fmt::format_string<Args...> format, Args&&... args) {
    if (log_level < level)
      return;

    TimePoint now = std::chrono::system_clock::now();
    uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
    
    std::tm time_tm = fmt::localtime(std::chrono::system_clock::to_time_t(now));

    std::string log = fmt::format(format, std::forward<Args>(args)...);
    fmt::print("{:%H:%M:%S}.{:03d}\033{}[{}]\033[0m {}\n", time_tm, (int)ms, str_color, prefix, log);
  }

  template <typename... Args>
  inline static void fatal(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::LOG_INFO, "FATAL", ERROR_COLOR, format, std::forward<Args>(args)...);
    std::exit(1);
  }

  template <typename... Args>
  inline static void error(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::LOG_INFO, "ERROR", ERROR_COLOR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void warning(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::LOG_WARNING, "WARNING", WARNING_COLOR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void info(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::LOG_INFO, "INFO", INFO_COLOR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void debug(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::LOG_DEBUG, "DEBUG", DEBUG_COLOR, format, std::forward<Args>(args)...);
  }

  template <typename... Args>
  inline static void verbose(fmt::format_string<Args...> format, Args&&... args) {
    log(LogLevel::LOG_VERBOSE, "VERBOSE", VERBOSE_COLOR, format, std::forward<Args>(args)...);
  }

private:
  inline static LogLevel log_level = LogLevel::LOG_INFO;
};

} // namespace ntf
