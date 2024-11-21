#pragma once

#include "../stl/common.hpp"

#include <fstream>
#include <sstream>

namespace ntf {

inline std::string file_contents(std::string path) {
  std::string out {};
  std::fstream fs{path};

  if (!fs.is_open()) {
    throw ntf::error{"File not found: {}", path};
  } else {
    std::ostringstream ss;
    ss << fs.rdbuf();
    out = ss.str();
  }

  fs.close();
  return out;
}

inline std::string file_dir(std::string path) {
  return path.substr(0, path.find_last_of('/'));
}

} // namespace ntf
