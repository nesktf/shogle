#pragma once

#include <string>

namespace ntf::res {

enum class texture_type {
  tex2d = 0,
  cubemap
};

enum class texture_filter {
  nearest = 0,
  linear
};

enum class texture_format {
  rgb = 0,
  rgba,
  mono,
};

struct spritesheet_loader;

struct texture_loader {
private:
  friend spritesheet_loader;
  texture_loader() = default;
public:
  texture_loader(std::string _path);
  ~texture_loader();

  texture_loader(texture_loader&&) noexcept;
  texture_loader(const texture_loader&) = delete;
  texture_loader& operator=(texture_loader&&) noexcept;
  texture_loader& operator=(const texture_loader&) = delete;

  std::string path {};
  int width {1}, height {1}, channels {0};
  texture_type type {texture_type::tex2d};
  texture_format format {texture_format::rgb};
  texture_filter filter {texture_filter::nearest};
  unsigned char* pixels[6] {nullptr};
};

}
