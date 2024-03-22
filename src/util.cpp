#include "util.hpp"
#include "log.hpp"

#include <fstream>
#include <sstream>

namespace ntf::shogle::util {

std::string file_contents(std::string path) {
  std::string out;
  std::fstream fs{path};
  if (!fs.is_open()) {
    Log::error("[Util] File not found: {}", path);
  } else {
    std::ostringstream ss;
    ss << fs.rdbuf();
    out = ss.str();
  }
  fs.close();
  return out;
}

} // namespace ntf::shogle::util
