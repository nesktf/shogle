#pragma once

#include <string>

namespace ntf::shogle {

class ShaderData {
public:
  ShaderData(const char* vertex_path, const char* fragmt_path);
  ~ShaderData() = default;

public:
  std::string vertex_src;
  std::string fragmt_src;
};

}
