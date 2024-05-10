#pragma once

#include <shogle/core/types.hpp>

#include <vector>
#include <string>
#include <unordered_map>

namespace ntf::res {

// texture
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
  unsigned char* pixels {nullptr};
};

// shader
struct shader_loader {
  shader_loader(std::string path);
  
  std::string path;
  std::string vert_src;
  std::string frag_src;
};

// script
struct script_loader {
  script_loader(std::string _path);

  std::string path;
  std::string content;
};

// model
struct model_loader {
  struct mesh {
    struct material {
      texture_loader texture;
      std::string uniform_name;
    };
    struct vertex {
      vec3 coord;
      vec3 normal;
      vec2 tex_coord;
    };
    std::vector<vertex> vertices;
    std::vector<uint> indices;
    std::vector<material> materials;
  };

  model_loader(std::string _path);

  std::string path;
  std::vector<mesh> meshes;
};

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

} // namespace ntf::res
