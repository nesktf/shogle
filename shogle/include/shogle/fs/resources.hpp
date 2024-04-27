#pragma once

#include <shogle/core/types.hpp>

#include <vector>
#include <string>
#include <unordered_map>

namespace ntf::fs {
// texture
enum class texture_type {
  tex2d = 0,
  tex3d
};

struct texture_loader {
  texture_loader(std::string _path);
  texture_loader() = default;
  ~texture_loader();

  texture_loader(texture_loader&&) noexcept;
  texture_loader(const texture_loader&) = delete;
  texture_loader& operator=(texture_loader&&) noexcept;
  texture_loader& operator=(const texture_loader&) = delete;

  std::string path {};
  int width {1}, height {1}, channels {0};
  texture_type t_dim {texture_type::tex2d};
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

// material
enum class material_type {
  diffuse = 0,
  specular
};

struct material_loader {
  material_loader(std::string _path);

  texture_loader tex;
  material_type type {material_type::diffuse};
};

// model
struct mesh_loader {
  struct vertex {
    vec3 coord;
    vec3 normal;
    vec2 tex_coord;
  };
  std::vector<vertex> vertices;
  std::vector<uint> indices;
  std::vector<material_loader> materials;
};

struct model_loader {
  model_loader(std::string _path);

  std::string path;
  std::vector<mesh_loader> meshes;
};

// spritesheet
struct sprite_data {
  size_t count;
  size_t x, y;
  size_t x0, y0;
  size_t dx, dy;
  size_t cols, rows;
};

struct spritesheet_loader {
  spritesheet_loader(std::string _path);

  std::string path;
  texture_loader tex;
  std::unordered_map<std::string, sprite_data> sprites;
};

} // namespace ntf::fs
