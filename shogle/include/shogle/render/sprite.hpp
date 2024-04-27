#pragma once

#include <shogle/render/res/texture.hpp>
#include <shogle/fs/res/spritesheet.hpp>

#include <string>
#include <unordered_map>

namespace ntf::render {

class Spritesheet : public Texture {
public:
  using data_t = fs::spritesheet_data;

public:
  Spritesheet(const Spritesheet::data_t* data);

public:
  std::unordered_map<std::string, fs::sprite_data> sprites;
};

} // namespace ntf::render
