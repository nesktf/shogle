#include <shogle/core.hpp>
#include <shogle/util/logger.hpp>

#include <fmt/chrono.h>
#include <fmt/printf.h>

namespace shogle {

namespace {

shogle::logger::log_level level = logger::LEVEL_INFO;

constexpr std::string_view level_pfxs[]{
  "ERROR", "WARNING", "INFO", "DEBUG", "VERBOSE",
};
constexpr std::string_view level_colors[]{
  "[0;31m", "[0;33m", "[0;34m", "[0;32m", "[0;37m",
};

auto get_time() {

  using TimePoint = std::chrono::system_clock::time_point;
  TimePoint now = std::chrono::system_clock::now();
  uint64_t ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

  return std::make_pair(std::chrono::system_clock::to_time_t(now), ms);
}

} // namespace

void logger::set_level(log_level level_) noexcept {
  level = level_;
}

logger::log_level logger::get_level() noexcept {
  return level;
}

void logger::_do_log(log_level level, std::string_view prefix, const std::string& str) {
  level = level > LEVEL_VERBOSE ? LEVEL_VERBOSE : level;
  const auto [time, ms] = get_time();
  const std::tm* time_tm = std::localtime(&time);
  fmt::print("[{:%H:%M:%S}.{:03d}]\033{}[{}]\033[0m[ShOGLE][{}]{}\n", *time_tm, (int)ms,
             level_colors[(int)level], level_pfxs[(int)level], prefix, str);
}

void logger::_do_log(log_level level, const std::string& str) {
  level = level > LEVEL_VERBOSE ? LEVEL_VERBOSE : level;
  const auto [time, ms] = get_time();
  const std::tm* time_tm = std::localtime(&time);
  fmt::print("[{:%H:%M:%S}.{:03d}]\033{}[{}]\033[0m {}\n", *time_tm, (int)ms,
             level_colors[(int)level], level_pfxs[(int)level], str);
}

} // namespace shogle

namespace shogle::impl {

[[noreturn]] void assert_failure(const char* cond, const char* func, const char* file, int line,
                                 const char* msg) {
  if (msg) {
    shogle::logger::log_prefix(logger::LEVEL_ERROR, "CORE", "{}:{}: {} assertion '{}' failed: {}",
                               file, line, func, cond, msg);
  } else {
    shogle::logger::log_prefix(logger::LEVEL_ERROR, "CORE", "{}:{}: {} assertion '{}' failed",
                               file, line, func, cond, msg);
  }
  SHOGLE_ABORT();
  __builtin_unreachable();
}

} // namespace shogle::impl
