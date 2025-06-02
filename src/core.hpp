#pragma once

#include <ntfstl/core.hpp>

#if defined(SHOGLE_ENABLE_INTERNAL_LOGS) && SHOGLE_ENABLE_INTERNAL_LOGS
#include <ntfstl/logger.hpp>
#define SHOGLE_LOG(_priority, _fmt, ...) \
  ::ntf::logger::_priority(_fmt, ##__VA_ARGS__)
  // ::ntf::logger::_priority("[ShOGLE]" _fmt, ##__VA_ARGS__)
#else
#define SHOGLE_LOG(_priority, _fmt, ...) NTF_NOOP
#endif
