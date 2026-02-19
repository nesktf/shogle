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
  static void _do_log(log_level level, const std::string& str);

public:
  template<typename... Args>
  static void log_prefix(log_level level, std::string_view pre, fmt::format_string<Args...> fmt,
                  Args&&... args) {
    if (get_level() < level) {
      return;
    }
    const auto str = fmt::format(fmt, std::forward<Args>(args)...);
    _do_log(level, pre, str);
  }

  template<typename... Args>
  static void log(log_level level, fmt::format_string<Args...> fmt,
                  Args&&... args) {
    if (get_level() < level) {
      return;
    }
    const auto str = fmt::format(fmt, std::forward<Args>(args)...);
    _do_log(level, str);
  }

  template<typename... Args>
  static void error(fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_ERROR, fmt, std::forward<Args>(args)...);
  }


  template<typename... Args>
  static void warning(fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_WARNING, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void info(fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_INFO, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void debug(fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_DEBUG, fmt, std::forward<Args>(args)...);
  }

  template<typename... Args>
  static void verbose(fmt::format_string<Args...> fmt, Args&&... args) {
    log(LEVEL_VERBOSE, fmt, std::forward<Args>(args)...);
  }
};

} // namespace shogle
