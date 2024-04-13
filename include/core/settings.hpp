#pragma once

#include "core/types.hpp"

#include <string>

namespace ntf {

class Settings {
public:
  Settings(int argc, char* argv[]);
  Settings(int argc, char* argv[], const char* path);

public:
  color3 clear_color {0.2f, 0.2f, 0.2f};
  size_t w_width {800};
  size_t w_height {600};
  std::string w_title {"shogle"};
};

} // namespace ntf
