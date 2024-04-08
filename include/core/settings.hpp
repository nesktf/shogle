#pragma once

#include <glm/vec3.hpp>

#include <string>

namespace ntf {

class Settings {
public:
  Settings(int argc, char* argv[]);
  Settings(int argc, char* argv[], const char* path);

public:
  glm::vec3 clear_color = {0.2f, 0.2f, 0.2f};
  unsigned int w_width = 800;
  unsigned int w_height = 600;
  std::string w_title = "shogle";
};

} // namespace ntf
