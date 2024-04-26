#include <shogle/util/fs.hpp>

#include <shogle/core/log.hpp>

#include <fstream>
#include <sstream>

namespace ntf::fs {

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

} // namespace ntf::fs