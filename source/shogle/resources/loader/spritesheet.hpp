#pragma once

#include <shogle/resources/loader/texture.hpp>

#include <unordered_map>

namespace ntf::res {

// spritesheet
struct spritesheet_loader {
  struct sprite {
    size_t count;
    size_t x, y;
    size_t x0, y0;
    size_t dx, dy;
    size_t cols, rows;
  };

  spritesheet_loader(std::string _path);

  std::string path;
  texture_loader tex;
  std::unordered_map<std::string, sprite> sprites;
};
}
