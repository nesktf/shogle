#include "log.hpp"

namespace ntf::shogle::log {

LogLevel log_level = LogLevel::LOG_INFO;

void set_level(LogLevel level) {
  log_level = level;
}

}
