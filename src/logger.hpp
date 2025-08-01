#pragma once

#include <shogle/core.hpp>

#include <string_view>
#include <algorithm>

#if defined(SHOGLE_ENABLE_INTERNAL_LOGS) && SHOGLE_ENABLE_INTERNAL_LOGS
#include <ntfstl/logger.hpp>
#define SHOGLE_LOG(_priority, _fmt, ...) \
  ::ntf::logger::_priority("[{}:{}] " _fmt, \
                           ::shogle::meta::parse_src_str(NTF_FILE), NTF_LINE \
                           __VA_OPT__(,) __VA_ARGS__)
#else
#define SHOGLE_LOG(_priority, _fmt, ...) NTF_NOOP
#endif

namespace shogle::meta {

consteval std::string_view parse_src_str(std::string_view file_str) {
  const char pos[] = {"src/"};
  std::string_view pos_str{&pos[0], 4};
  auto iter = std::search(file_str.begin(), file_str.end(),
                          pos_str.begin(), pos_str.end());
  auto prefix_len = iter - std::begin(file_str) + 4;
  auto out_len = file_str.size() - prefix_len;
  return {file_str.data()+prefix_len, out_len};
}

} // namespace shogle::meta
