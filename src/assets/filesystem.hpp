#include "./types.hpp"

namespace ntf {

inline optional<std::string> file_contents(std::string_view path) {
  std::string out {};
  std::fstream fs{path.data()};

  if (!fs.is_open()) {
    return nullopt;
  }

  std::ostringstream ss;
  ss << fs.rdbuf();
  out = ss.str();

  fs.close();
  return out;
}

inline optional<std::string> file_dir(std::string_view path) {
  auto pos = path.find_last_of('/');
  if (pos == std::string::npos) {
    return nullopt;
  }
  return std::string{path.substr(0, pos)};
}

} // namespace ntf
