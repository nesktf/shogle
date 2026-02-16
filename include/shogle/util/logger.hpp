#pragma once

#include <fmt/core.h>

namespace shogle {

class logger {
public:
  enum log_level {
    LEVEL_ERROR = 0,
    LEVEL_WARNING,
    LEVEL_INFO,
    LEVEL_DEBUG,
    LEVEL_VERBOSE,
  };

public:
  static void set_level(log_level level) noexcept;
  static log_level get_level() noexcept;

private:
  static void _do_log(log_level level, std::string_view prefix, const std::string& str);

public:
  template<typename... Args>
  static void log(log_level level, std::string_view pre, fmt::format_string<Args...> fmt,
                  Args&&... args) {
    if (get_level() < level) {
      return;
    }
    const auto str = fmt::format(fmt, std::forward<Args>(args)...);
    _do_log(level, pre, str);
  }

  template<typename... Args>
  static void log_error(std::string_view pre, fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_ERROR, pre, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void log_warning(std::string_view pre, fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_WARNING, pre, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void log_info(std::string_view pre, fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_INFO, pre, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void log_debug(std::string_view pre, fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_DEBUG, pre, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void log_verbose(std::string_view pre, fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_VERBOSE, pre, fmt, std::forward<Args>(args)...);
  }
};

} // namespace shogle
