#pragma once

#include <string>

namespace ntf::res {
// shader
struct shader_loader {
  shader_loader(std::string path);
  
  std::string path;
  std::string vert_src;
  std::string frag_src;
};

}
