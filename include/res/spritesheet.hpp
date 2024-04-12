#pragma once

#include "res/texture.hpp"

#include <string>
#include <unordered_map>

namespace ntf {

struct SpriteData {
  size_t count;
  size_t x, y;
  size_t x0, y0;
  size_t dx, dy;
  size_t cols, rows;
};

// Spritesheet::data_t
class SpritesheetData {
public: // Resource data can be copied but i don't think is a good idea
  SpritesheetData(std::string path);
  ~SpritesheetData() = default;

  SpritesheetData(SpritesheetData&&) = default;
  SpritesheetData& operator=(SpritesheetData&&) = default;

  SpritesheetData(const SpritesheetData&) = delete;
  SpritesheetData& operator=(const SpritesheetData&) = delete;

  std::unique_ptr<Texture::data_t> tex_data;
  std::unordered_map<std::string, SpriteData> sprites;
};

// Spritesheet
class Spritesheet : public Texture {
public:
  using data_t = SpritesheetData;

public:
  Spritesheet(const Spritesheet::data_t* data);

  std::unordered_map<std::string, SpriteData> sprites;
};

} // namespace ntf
