#include "core/logger.hpp"

namespace ntf::shogle::logger {

LogLevel log_level = LogLevel::LOG_INFO;

void set_level(LogLevel level) {
  log_level = level;
}

}
