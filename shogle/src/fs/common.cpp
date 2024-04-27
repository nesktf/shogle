#include <shogle/fs/common.hpp>
#include <shogle/core/error.hpp>

#include <fmt/format.h>

#include <fstream>
#include <sstream>

namespace ntf::fs {

std::string file_contents(std::string path) {
  std::string out {};
  std::fstream fs{path};

  if (!fs.is_open()) {
    throw ntf::error(fmt::format("File not found: {}", path).c_str());
  } else {
    std::ostringstream ss;
    ss << fs.rdbuf();
    out = ss.str();
  }

  fs.close();
  return out;
}

std::string file_dir(std::string path) {
  return path.substr(0, path.find_last_of('/'));
}

} // namespace ntf::fs
