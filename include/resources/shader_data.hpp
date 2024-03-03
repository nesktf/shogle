#pragma once

#include <string>

namespace ntf::shogle {

struct ShaderData {
  ShaderData(const char* vertex_path, const char* fragmt_path);

  std::string vertex_src;
  std::string fragmt_src;
};

}
