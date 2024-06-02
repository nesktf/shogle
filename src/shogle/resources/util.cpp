#include <shogle/resources/util.hpp>

#include <shogle/core/error.hpp>

#include <fstream>
#include <sstream>

namespace ntf::shogle::resources {

std::string file_contents(std::string path) {
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

std::string file_dir(std::string path) {
  return path.substr(0, path.find_last_of('/'));
}

void async_loader::do_requests() {
  while (!_req.empty()) {
    auto req_callback = std::move(_req.front());
    _req.pop();
    req_callback();
  }
}

} // namespace ntf::shogle::resources
